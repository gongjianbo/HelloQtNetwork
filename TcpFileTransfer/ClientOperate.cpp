#include "ClientOperate.h"

#include <QFileInfo>

ClientOperate::ClientOperate(QObject *parent)
    : QObject(parent)
{
    initOperate();
}

ClientOperate::~ClientOperate()
{
    doDisconnect();
}

QString ClientOperate::getFilePath() const
{
    QMutexLocker locker(&dataMutex);
    return filePath;
}

void ClientOperate::setFilePath(const QString &path)
{
    QMutexLocker locker(&dataMutex);
    filePath=path;
}

bool ClientOperate::isConnected() const
{
    QMutexLocker locker(&dataMutex);
    return connectState;
}

void ClientOperate::setConnected(bool connected)
{
    QMutexLocker locker(&dataMutex);
    connectState=connected;
}

void ClientOperate::connectTcp(const QString &address, quint16 port)
{
    if(socket->state()==QAbstractSocket::UnconnectedState){
        //连接服务器
        socket->connectToHost(QHostAddress(address),port);
    }else{
        emit logMessage("socket->state() != QAbstractSocket::UnconnectedState");
    }
}

void ClientOperate::disconnectTcp()
{
    doDisconnect();
}

void ClientOperate::startFileTransfer()
{
    //之前如果打开了先释放
    doCloseFile();
    if(!socket->isValid())
        return;
    const QString file_path=getFilePath();
    //无效路径
    if(file_path.isEmpty() || !QFile::exists(file_path)){
        emit logMessage("无效的文件路径"+file_path);
        return;
    }
    file=new QFile(this);
    file->setFileName(file_path);
    //打开失败
    if(!file->open(QIODevice::ReadOnly)){
        doCloseFile();
        emit logMessage("打开文件失败"+file_path);
        return;
    }

    sendSize=0;
    fileSize=file->size();
    if(fileSize<0)
        fileSize=0;
    //大小高位字节顺序在前
    char file_size[4]={0};
    const quint64 data_size=fileSize; //有符号转无符号，会被截断
    file_size[3]=data_size>>0%0x100;
    file_size[2]=data_size>>8%0x100;
    file_size[1]=data_size>>16%0x100;
    file_size[0]=data_size>>24;
    //把文件大小和文件名发送给服务端，然后等待确认命令的返回
    QFileInfo info(file_path);
    sendData(0x01,QByteArray(file_size,4)+info.fileName().toUtf8());
}

void ClientOperate::cancelFileTransfer()
{
    //关闭文件
    doCancel();
    //发送停止传输指令
    sendData(0x04,QByteArray());
}

void ClientOperate::initOperate()
{
    socket=new QTcpSocket(this);

    //收到数据，触发readyRead
    connect(socket,&QTcpSocket::readyRead,[this]{
        //没有可读的数据就返回
        if(socket->bytesAvailable()<=0)
            return;
        //读取数据
        operateReceiveData(socket->readAll());
    });

    //连接状态改变
    connect(socket,&QTcpSocket::connected,[this]{
        setConnected(true);
        emit connectStateChanged(true);
        emit logMessage(QString("已连接服务器 [%1:%2]")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
    });
    connect(socket,&QTcpSocket::disconnected,[this]{
        setConnected(false);
        emit connectStateChanged(false);
        emit logMessage(QString("与服务器连接已断开 [%1:%2]")
                        .arg(socket->peerAddress().toString())
                        .arg(socket->peerPort()));
    });

    timer=new QTimer(this);

    //通过定时器来控制数据发送
    connect(timer,&QTimer::timeout,[this]{
        if(!socket->isValid()){
            doCancel();
            emit logMessage("Socket不可操作，发送终止");
            return;
        }
        if(!file||!file->isOpen()){
            doCancel();
            emit logMessage("文件操作失败，发送终止");
            return;
        }

        const qint64 read_size=file->read(fileBuffer,4096);
        //socket->write(fileBuffer,read_size);
        sendFile(fileBuffer,read_size);
        sendSize+=read_size;
        file->seek(sendSize);
        if(!socket->waitForBytesWritten()){
            doCancel();
            emit logMessage("文件发送超时，发送终止");
            return;
        }
        //避免除零
        if(fileSize>0){
            emit progressChanged(sendSize*100/fileSize);
        }
        if(sendSize>=fileSize){
            doCancel();
            emit logMessage("文件发送完成");
            emit progressChanged(100);
            sendData(0x03,QByteArray());
            return;
        }
    });
}

void ClientOperate::doDisconnect()
{
    //断开socket连接，释放资源
    socket->abort();
    doCloseFile();
}

void ClientOperate::doCloseFile()
{
    if(file){
        file->close();
        delete file;
        file=nullptr;
    }
}

void ClientOperate::doCancel()
{
    timer->stop();
    if(file){
        //关闭文件
        doCloseFile();
    }
}

void ClientOperate::sendData(char type,const QByteArray &data)
{
    //传输协议
    //帧结构：帧头4+帧长2+帧类型1+帧数据N+帧尾2（没有校验段，懒得写）
    //帧头：4字节定值 0x0F 0xF0 0x00 0xFF
    //帧长：2字节数据段长度值 arr[4]*0x100+arr[5] 前面为高位后面为低位
    //帧类型：1字节
    //- 0x01 准备发送文件，后跟四字节文件长度和N字节utf8文件名，长度计算同帧长一样前面为高位后面为低位
    //- 0x02 文件数据
    //- 0x03 发送结束
    //- 0x04 取消发送
    //（服务端收到0x01 0x03开始和结束发送两个命令要进行应答，回同样的命令码无数据段）
    //帧尾：2字节定值 0x0D 0x0A
    if(!socket->isValid())
        return;
    frameHead[6]=type;
    const quint64 data_size=data.count();
    frameHead[5]=data_size%0x100;
    frameHead[4]=data_size/0x100;

    //发送头+数据+尾
    socket->write(frameHead,7);
    socket->write(data);
    socket->write(frameTail,2);
}

void ClientOperate::sendFile(const char *data, int size)
{
    if(!socket->isValid())
        return;
    frameHead[6]=(char)0x02;
    const quint64 data_size=size;
    frameHead[5]=data_size%0x100;
    frameHead[4]=data_size/0x100;

    //发送头+数据+尾
    socket->write(frameHead,7);
    socket->write(data,size);
    socket->write(frameTail,2);
}

void ClientOperate::operateReceiveData(const QByteArray &data)
{
    static QByteArray frame_head=QByteArray(frameHead,4);
    //这里只是简单的处理，所以用了QByteArray容器做缓存
    dataTemp+=data;

    //处理数据
    while(true){
        //保证以帧头为起始
        while(!dataTemp.startsWith(frame_head)&&dataTemp.size()>4){
            dataTemp.remove(0,1); //左边移除一字节
        }
        //小于最小帧长
        if(dataTemp.size()<7+2)
            return;
        //取数据段长度，这里没有判断长度有效性
        const int data_size=uchar(dataTemp[4])*0x100+uchar(dataTemp[5]);
        if(dataTemp.size()<7+2+data_size)
            return;
        //帧尾不一致，无效数据--这里懒得写校验位了
        if(memcmp(dataTemp.constData()+7+data_size,frameTail,2)!=0){
            dataTemp.clear();
            return;
        }
        //取数据类型
        const char type=dataTemp[6];
        switch(type)
        {
        case 0x01: //开始发送数据应答
            timer->start(0);
            emit logMessage("服务器已准备好接收数据，开始发送"+getFilePath());
            break;
        case 0x03: //发送数据完成应答
        {
            //1成功，0失败
            const bool result=(dataTemp[7]==(char)0x01);
            emit logMessage(QString("服务器文件接收完毕，发送")+(result?"成功":"失败"));
        }
            break;
        case 0x04: //服务端取消发送
            doCancel();
            emit logMessage("服务器取消发送，发送终止");
            break;
        default: break;
        }
        //移除处理完的字节
        dataTemp.remove(0,7+2+data_size);
    }
}
