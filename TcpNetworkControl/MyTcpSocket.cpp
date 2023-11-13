#include "MyTcpSocket.h"
#include <stdlib.h>
#include <memory.h>
#include <QHostAddress>
#include <QtGlobal>
#include <QDebug>

static int number;

MyTcpSocket::MyTcpSocket(qintptr descriptor, QObject *parent)
    : QObject(parent), socketDescriptor(descriptor)
{
    qDebug() << "init socket" << number++;
}

MyTcpSocket::~MyTcpSocket()
{
    socket->abort();
    keepTimer->stop();
    qDebug() << "free socket" << --number;
}

void MyTcpSocket::init()
{
    keepTimer = new QTimer(this);
    keepTimer->setInterval(1000 + (rand() % 500));
    connect(keepTimer, &QTimer::timeout, this, [this](){
        if (!doKeep) {
            keepTimer->stop();
            return;
        }
        // 长时间没收到心跳，断开
        if (--keepCounter < 0) {
            disconnectSocket();
            return;
        }
        // 双端都发心跳包，不响应
        keepAlive();
    });

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, [this](){
        keepCounter = 5;
        cache.clear();
        if (doKeep) {
            keepTimer->start();
        }
        emit connectStateChanged(true);
    });
    connect(socket, &QTcpSocket::disconnected, this, [this](){
        keepTimer->stop();
        cache.clear();
        emit connectStateChanged(false);
    });
    connect(socket, &QTcpSocket::readyRead, this, &MyTcpSocket::recvData);

    if (socketDescriptor) {
        socket->setSocketDescriptor(socketDescriptor);
        // 相当于已连接
        keepCounter = 5;
        cache.clear();
        if (doKeep) {
            keepTimer->start();
        }
    }
}

void MyTcpSocket::connectServer(const QString &ip, quint16 port, bool heart)
{
    if (socket->state() != QTcpSocket::UnconnectedState)
        socket->abort();
    doKeep = heart;
    socket->connectToHost(ip, port);
    socket->waitForConnected(3000);
}

void MyTcpSocket::disconnectSocket()
{
    socket->abort();
}

bool MyTcpSocket::isConnected()
{
    return socket->state() == QTcpSocket::ConnectedState;
}

void MyTcpSocket::keepAlive()
{
    ProtocolFrame frame;
    frame.command = CC_KeepAlive;
    frame.error = CE_NoErr;
    sendFrame(frame);
}

bool MyTcpSocket::waitWritten()
{
    while (isConnected()) {
        if (socket->bytesToWrite() == 0) {
            return true;
        }
        socket->waitForBytesWritten(1000);
    }
    return false;
}

void MyTcpSocket::sendFrame(const ProtocolFrame &frame)
{
    if (!isConnected()) {
        return;
    }
    bool ok = false;
    QByteArray frame_data = ProtocolParser::pack(frame, &ok);
    if (!ok) {
        return;
    }
    socket->write(ProtocolParser::FRAME_HEAD, 4);
    unsigned int frame_len = 16 + frame_data.size();
    socket->write(ProtocolParser::uint32ToBytes(frame_len));
    socket->write(frame_data);
    socket->write(ProtocolParser::FRAME_EMPTY, 4);
    socket->write(ProtocolParser::FRAME_TAIL, 4);
    socket->waitForBytesWritten(1000);
}

void MyTcpSocket::recvData()
{
    if (socket->bytesAvailable() <= 0)
        return;
    keepCounter = 5;
    cache += socket->readAll();

    // 处理数据
    while (true)
    {
        // 判断帧头
        while (cache.size() > 4 && memcmp(ProtocolParser::FRAME_HEAD, cache.data(), 4) != 0) {
            cache.remove(0, 1);
        }
        if (cache.size() < 16)
            return;
        // 帧长
        const unsigned int data_size = ProtocolParser::uint32FromBytes(cache.mid(4, 4));
        if ((unsigned int)cache.size() < data_size)
            return;
        // 判断帧尾
        if (memcmp(ProtocolParser::FRAME_TAIL, cache.data() + data_size - 4, 4) != 0) {
            cache.clear();
            return;
        }
        // 校验，略
        bool ok = false;
        ProtocolFrame frame = ProtocolParser::unpack(cache.mid(8, data_size - 16), &ok);
        // command=0是心跳包
        if (ok && frame.command != CC_KeepAlive) {
            emit recvFrame(frame);
        }
        // 移除已处理数据
        cache.remove(0, data_size);
    }
}
