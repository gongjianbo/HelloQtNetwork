#include "ServerOperate.h"

#include <QTextStream>
#include <QDebug>

ServerOperate::ServerOperate(QObject *parent)
    : QObject(parent)
{
    initServer();
}

ServerOperate::~ServerOperate()
{
    QList<QThread *> thread_list=threadList.keys();
    qDebug()<<"thread count"<<thread_list.count();
    threadList.clear();
    for(QThread *thread:thread_list){
        thread->quit();
        thread->wait();
    }
    qDeleteAll(thread_list);
}

void ServerOperate::initServer()
{
    //只是做演示，没有对流程进行封装
    server=new MyTcpServer(this);
    //我重载了server的incomingConnection接口，将socket handle传了出来
    connect(server,&MyTcpServer::newConnectionHandle,
            this,[=](qintptr handle){
        //构造一个qtcpsocket，然后move到线程中收发数据
        //（可以封装一个类，我这里简化操作）
        QTcpSocket *socket=new QTcpSocket;
        socket->setSocketDescriptor(handle);
        const quint16 peer_port=socket->peerPort();
        //qDebug()<<"new connect"<<peer_port;
        QThread *thread=new QThread;
        threadList.insert(thread,peer_port);
        socket->moveToThread(thread);

        //线程退出时释放
        connect(thread,&QThread::finished,socket,&QTcpSocket::deleteLater);
        connect(socket,&QTcpSocket::readyRead,this,[=]{
            while(socket->bytesAvailable()>0){
                //没有考虑编码，也没考虑数据帧完整性
                emit clientMessage(peer_port,socket->readAll());
            }
        });
        connect(socket,&QTcpSocket::disconnected,this,[=]{
            //通知ui该连接已断开
            //const int index=threadList.indexOf(thread);
            emit clientDisconnected(peer_port);
            if(thread){
                thread->quit();
                thread->wait();
                threadList.remove(thread);
                thread->deleteLater();
            }
        });
        connect(thread,&QThread::started,[=]{
            //started信号在QThread::run之前触发的，处理同一个函数中，所以线程也都是在子线程
            //没有填接收者对象，槽函数也是在发送者同一线程执行，获取到的线程id也就是发送者线程的id

            //通知ui有新的连接
            emit clientConnected(peer_port,QString::asprintf("%p",QThread::currentThreadId()));
        });
        thread->start();
    });
    server->listen(QHostAddress("127.0.0.1"),23456);
}

void ServerOperate::closeConnect(quint16 port)
{
    QThread *thread=threadList.key(port,nullptr);
    if(thread){
        thread->quit();
        thread->wait();
        threadList.remove(thread);
        thread->deleteLater();
    }
}
