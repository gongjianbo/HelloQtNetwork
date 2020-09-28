#ifndef SERVEROPERATE_H
#define SERVEROPERATE_H

#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include "MyTcpServer.h"

//server管理，一个连接一个线程
//（只是演示基本操作，拿到socket handle怎么处理都行）
class ServerOperate : public QObject
{
    Q_OBJECT
public:
    explicit ServerOperate(QObject *parent = nullptr);
    ~ServerOperate();

private:
    void initServer();

signals:
    //暂时用端口号来遍历查找对应的连接
    void clientConnected(quint16 port,const QString &id);
    void clientDisconnected(quint16 port);
    void clientMessage(quint16 port,const QString &msg);

public slots:
    void closeConnect(quint16 port);

private:
    //自定义的server类
    MyTcpServer *server;
    //线程列表
    QMap<QThread *,quint16> threadList;
};

#endif // SERVEROPERATE_H
