#include "ClientOperate.h"

ClientOperate::ClientOperate(QObject *parent)
    : QObject(parent)
{
    initOperate();
}

ClientOperate::~ClientOperate()
{
    doDisconnect();
}

QString ClientOperate::getFilePath() const
{
    QMutexLocker locker(&dataMutex);
    return filePath;
}

void ClientOperate::setFilePath(const QString &path)
{
    QMutexLocker locker(&dataMutex);
    filePath=path;
}

bool ClientOperate::isConnected() const
{
    QMutexLocker locker(&dataMutex);
    return (socket->state()==QAbstractSocket::ConnectedState);
}

void ClientOperate::connectTcp(const QString &address, quint16 port)
{
    if(socket->state()==QAbstractSocket::UnconnectedState){
        //连接服务器
        socket->connectToHost(QHostAddress(address),port);
    }else{
        emit logMessage("socket->state() != QAbstractSocket::UnconnectedState");
    }
}

void ClientOperate::disconnectTcp()
{
    doDisconnect();
}

void ClientOperate::startFileTransfer()
{
    socket->write("test");
}

void ClientOperate::cancelFileTransfer()
{
    if(file){
        //发送停止传输指令
        //关闭文件
        file->close();
        file->deleteLater();
        file=nullptr;
    }
}

void ClientOperate::initOperate()
{
    socket=new QTcpSocket(this);

    //收到数据，触发readyRead
    connect(socket,&QTcpSocket::readyRead,[this]{
        //没有可读的数据就返回
        if(socket->bytesAvailable()<=0)
            return;
        //读取数据
        operateReceiveData(socket->readAll());
    });

    //连接状态改变
    connect(socket,&QTcpSocket::connected,[this]{
        emit connectStateChanged(true);
        emit logMessage(QString("已连接服务器 [%1:%2]")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
    });
    connect(socket,&QTcpSocket::disconnected,[this]{
        emit connectStateChanged(false);
        emit logMessage(QString("与服务器连接已断开 [%1:%2]")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
    });
}

void ClientOperate::doDisconnect()
{
    //断开socket连接，释放资源
    socket->abort();
    if(file){
        file->close();
    }
}

void ClientOperate::operateReceiveData(const QByteArray &data)
{
    //这里只是简单的处理，所以用了QByteArray容器做缓存
    //dataTemp+=data;
    emit progressChanged(data.count());
}
