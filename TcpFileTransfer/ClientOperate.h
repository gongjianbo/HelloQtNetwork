#ifndef CLIENTOPERATE_H
#define CLIENTOPERATE_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>

//客户端socket处理--客户端作为发送
class ClientOperate : public QObject
{
    Q_OBJECT
public:
    explicit ClientOperate(QObject *parent = nullptr);
    ~ClientOperate();

    //get/set 文件路径、连接状态等变量，使用了互斥锁
    QString getFilePath() const;
    void setFilePath(const QString &path);

    bool isConnected() const;

signals:
    //操作记录发送到ui显示
    void logMessage(const QString &msg);
    //连接状态改变
    void connectStateChanged(bool isConnect);
    //接收进度0-100
    void progressChanged(int value);

public slots:
    //连接
    void connectTcp(const QString &address,quint16 port);
    //断开连接
    void disconnectTcp();
    //传输文件
    void startFileTransfer();
    //取消文件传输
    void cancelFileTransfer();

private:
    void initOperate();
    void doDisconnect();
    void operateReceiveData(const QByteArray &data);

private:
    //用来锁文件路径、连接状态等
    mutable QMutex dataMutex;
    //文件存储路径
    QString filePath;
    //地址和端口
    QString address;
    quint16 port;
    //套接字
    QTcpSocket *socket=nullptr;
    //文件操作
    QFile *file=nullptr;
    //接收缓存
    QByteArray dataTemp;
};

#endif // CLIENTOPERATE_H
