#pragma once
#include <QTcpServer>
#include <QHash>
#include <QThread>
#include "MyTcpSocket.h"

/**
 * @brief TcpServer 封装
 * @author 龚建波
 * @date 2023-11-01
 */
class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

    // 客户端发来的命令，会在线程中回调该接口
    virtual void onRecvFrame(MyTcpSocket *socket, const ProtocolFrame &frame);

protected:
    // 有新的连接
    void incomingConnection(qintptr descriptor) override;

signals:
    // 发送给所有客户端
    void syncAll(const ProtocolFrame &frame);

protected:
    // 保存连进来的socket列表
    QHash<QThread*, MyTcpSocket*> socketTable;
};
