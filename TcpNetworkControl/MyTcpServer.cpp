#include "MyTcpServer.h"
#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer(parent)
{
    qDebug() << "init server";
}

MyTcpServer::~MyTcpServer()
{
    QList<QThread *> thread_list = socketTable.keys();
    socketTable.clear();
    for (QThread *thread : qAsConst(thread_list)) {
        thread->quit();
        thread->wait();
    }
    qDeleteAll(thread_list);
    qDebug() << "free server";
}

void MyTcpServer::onRecvFrame(MyTcpSocket *, const ProtocolFrame &)
{
}

void MyTcpServer::incomingConnection(qintptr descriptor)
{
    // 构造一个socket，然后move到线程中
    MyTcpSocket *socket = new MyTcpSocket(descriptor);

    QThread *thread = new QThread;
    socketTable.insert(thread, socket);
    socket->moveToThread(thread);

    // 线程退出时释放
    connect(thread, &QThread::finished, socket, &QTcpSocket::deleteLater);
    connect(this, &MyTcpServer::syncAll, socket, &MyTcpSocket::sendFrame);
    connect(socket, &MyTcpSocket::recvFrame, [=](const ProtocolFrame &frame){
        onRecvFrame(socket, frame);
    });
    connect(socket, &MyTcpSocket::connectStateChanged, this, [=](bool connected){
        if (thread && !connected) {
            thread->quit();
            thread->wait();
            socketTable.remove(thread);
            thread->deleteLater();
        }
    });
    connect(thread, &QThread::started, socket, &MyTcpSocket::init);
    thread->start();
}
