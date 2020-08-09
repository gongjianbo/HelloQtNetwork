#ifndef SERVEROPERATE_H
#define SERVEROPERATE_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>

//服务端socket处理--服务端作为接收
class ServerOperate : public QObject
{
    Q_OBJECT
public:
    explicit ServerOperate(QObject *parent = nullptr);
    ~ServerOperate();

    //get/set 文件路径、监听状态等变量，使用了互斥锁
    QString getFilePath() const;
    void setFilePath(const QString &path);

    bool isListening() const;

signals:
    //操作记录发送到ui显示
    void logMessage(const QString &msg);
    //服务端监听状态
    void listenStateChanged(bool isListen);
    //接收进度0-100
    void progressChanged(int value);

public slots:
    //server监听
    void listen(const QString &address,quint16 port);
    //server取消监听
    void dislisten();
    //取消文件传输
    void cancelFileTransfer();

private:
    //初始化
    void initOperate();
    //把槽对应的实际操作分离出来是为了复用，这样便于组合
    void doDislisten();
    void doCloseFile();
    void doCancel();
    bool readyReceiveFile(qint64 size,const QString &filename);
    void onReceiveFile(const char *data,qint64 size);
    void sendData(char type,const QByteArray &data);
    void operateReceiveData(const QByteArray &data);

private:
    //用来锁文件路径、监听状态等变量
    mutable QMutex dataMutex;
    //文件存储路径
    QString filePath;
    //地址和端口
    QString address;
    quint16 port;
    //套接字，本demo只允许一个客户端连接
    QTcpServer *server = nullptr;
    QTcpSocket *socket = nullptr;
    //文件操作
    QFile *file = nullptr;
    //文件长度
    qint64 fileSize = 0;
    //已接收文件长度
    qint64 receiveSize = 0;
    //接收缓存，实际操作时还是用char*好点
    QByteArray dataTemp;

    //帧头+长度+类型
    char frameHead[7] = { 0x0F, (char)0xF0, 0x00, (char)0xFF, 0x00, 0x00, 0x00 };
    //帧尾
    char frameTail[2] = { 0x0D, 0x0A };
};

#endif // SERVEROPERATE_H
