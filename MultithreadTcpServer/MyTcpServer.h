#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>

//重载QTcpServer的incomingConnection，将socket handle转发出去
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr handle) override;

signals:
    void newConnectionHandle(qintptr handle);
};

#endif // MYTCPSERVER_H
