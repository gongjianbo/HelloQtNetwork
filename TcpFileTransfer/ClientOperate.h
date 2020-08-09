#ifndef CLIENTOPERATE_H
#define CLIENTOPERATE_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QTimer>

//客户端socket处理--客户端作为发送
//帧没有数据校验字段
//没有流量控制
//没有错误数据段重发机制
//没有确认返回超时机制
//没有流程状态记录，如果当前正在发送却收到开始发送的错误命令，就乱套了
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
    void setConnected(bool connected);

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
    //初始化
    void initOperate();
    //把槽对应的实际操作分离出来是为了复用，这样便于组合
    void doDisconnect();
    void doCloseFile();
    void doCancel();
    void sendData(char type,const QByteArray &data);
    void sendFile(const char *data,int size);
    void operateReceiveData(const QByteArray &data);

private:
    //用来锁文件路径、连接状态等
    mutable QMutex dataMutex;
    //文件存储路径
    QString filePath;
    //地址和端口
    QString address;
    quint16 port;
    //连接状态
    bool connectState = false;
    //套接字
    QTcpSocket *socket = nullptr;
    //文件操作
    QFile *file = nullptr;
    //发送数据的定时器
    QTimer *timer = nullptr;
    //发送的字节数，因为Qt接口是int64，本来想用无符号类型
    qint64 sendSize=0;
    //文件大小
    qint64 fileSize=0;
    //接收数据的缓存，实际操作时还是用char*好点
    QByteArray dataTemp;

    //读取文件到缓冲区
    char fileBuffer[4096]={0};
    //帧头+长度+类型
    char frameHead[7] = { 0x0F, (char)0xF0, 0x00, (char)0xFF, 0x00, 0x00, 0x00 };
    //帧尾
    char frameTail[2] = { 0x0D, 0x0A };
};

#endif // CLIENTOPERATE_H
