#include "MyTcpServer.h"

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer(parent)
{

}

void MyTcpServer::incomingConnection(qintptr handle)
{
    //参照默认实现，可以把这个handle转发出去
    emit newConnectionHandle(handle);

    //目前我想到的是，
    //要么把handle或者tcpsocket转发出去，
    //要么在server类里直接管理这些thread和socket
    //多线程可以是一个socket一个线程，也可以放线程池轮询，
    //这个demo我打算就直接来一个连接就创建一个线程
}

//QTcpServer默认实现
//void QTcpServer::incomingConnection(qintptr socketDescriptor)
//{
//#if defined (QTCPSERVER_DEBUG)
//    qDebug("QTcpServer::incomingConnection(%i)", socketDescriptor);
//#endif
//    QTcpSocket *socket = new QTcpSocket(this);
//    socket->setSocketDescriptor(socketDescriptor);
//    addPendingConnection(socket);
//}
//
//void QTcpServer::addPendingConnection(QTcpSocket* socket)
//{
//    d_func()->pendingConnections.append(socket);
//}
