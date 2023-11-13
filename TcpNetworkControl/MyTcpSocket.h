#pragma once
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include "ProtocolParser.h"

/**
 * @brief TcpSocket 封装
 * @author 龚建波
 * @date 2023-11-01
 */
class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(qintptr descriptor = 0, QObject *parent = nullptr);
    ~MyTcpSocket();

    // 初始化
    void init();

    // 连接，客户端连接服务端使用
    void connectServer(const QString &ip, quint16 port, bool heart = true);
    // 断开
    void disconnectSocket();
    // 判断连接状态
    bool isConnected();

    // 发送心跳包
    void keepAlive();
    // 等待发送完成
    bool waitWritten();

signals:
    // 连接状态
    void connectStateChanged(bool connected);
    // 收到命令
    void recvFrame(const ProtocolFrame &frame);

public slots:
    // 发送命令
    void sendFrame(const ProtocolFrame &frame);
    // 接收网络数据
    void recvData();

protected:
    qintptr socketDescriptor{0};
    // 网络
    QTcpSocket *socket{nullptr};
    // 缓存数据流
    QByteArray cache;

    // 心跳定时器
    QTimer *keepTimer{nullptr};
    // 心跳计数
    int keepCounter{0};
    // 是否发心跳
    bool doKeep{true};
};

