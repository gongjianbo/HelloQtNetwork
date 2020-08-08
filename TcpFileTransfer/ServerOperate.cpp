#include "ServerOperate.h"

ServerOperate::ServerOperate(QObject *parent)
    : QObject(parent)
{
    initOperate();
}

ServerOperate::~ServerOperate()
{
    dislisten();
}

QString ServerOperate::getFilePath() const
{
    QMutexLocker locker(&dataMutex);
    return filePath;
}

void ServerOperate::setFilePath(const QString &path)
{
    QMutexLocker locker(&dataMutex);
    filePath=path;
}

bool ServerOperate::isListening() const
{
    QMutexLocker locker(&dataMutex);
    return server->isListening();
}

void ServerOperate::listen(const QString &address, quint16 port)
{
    if(server->isListening())
        doDislisten();
    //启动监听
    const bool result=server->listen(QHostAddress(address),port);
    emit listenStateChanged(result);
    emit logMessage(result?"服务启动成功":"服务启动失败");
}

void ServerOperate::dislisten()
{
    doDislisten();
    emit listenStateChanged(false);
    emit logMessage("服务关闭");
}

void ServerOperate::cancelFileTransfer()
{
    if(file){
        //发送停止传输指令
        //关闭文件
        file->close();
        file->deleteLater();
        file=nullptr;
    }
}

void ServerOperate::initOperate()
{
    server=new QTcpServer(this);
    //监听到新的客户端连接请求
    connect(server,&QTcpServer::newConnection,this,[this]{
        //如果有新的连接就取出
        while(server->hasPendingConnections())
        {
            //nextPendingConnection返回下一个挂起的连接作为已连接的QTcpSocket对象
            QTcpSocket *new_socket=server->nextPendingConnection();
            emit logMessage(QString("新的客户端连接 [%1:%2]")
                            .arg(new_socket->peerAddress().toString())
                            .arg(new_socket->peerPort()));
            //demo只支持一个连接，多余的释放掉
            if(socket){
                new_socket->abort();
                new_socket->deleteLater();
                emit logMessage("目前已有客户端连接，新连接已释放");
                continue;
            }else{
                socket=new_socket;
            }

            //收到数据，触发readyRead
            connect(socket,&QTcpSocket::readyRead,[this]{
                //没有可读的数据就返回
                if(socket->bytesAvailable()<=0)
                    return;
                //读取数据
                operateReceiveData(socket->readAll());
            });

            //连接断开，销毁socket对象
            connect(socket,&QTcpSocket::disconnected,[this]{
                emit logMessage(QString("客户端连接已断开 [%1:%2]")
                                .arg(socket->peerAddress().toString())
                                .arg(socket->peerPort()));
                socket->deleteLater();
                socket=nullptr;
            });
        }
    });
}

void ServerOperate::doDislisten()
{
    //关闭服务，断开socket连接，释放资源
    server->close();
    if(socket){
        socket->abort();
    }
    if(file){
        file->close();
    }
}

void ServerOperate::operateReceiveData(const QByteArray &data)
{
    //这里只是简单的处理，所以用了QByteArray容器做缓存
    //dataTemp+=data;
    emit progressChanged(data.count());
}
