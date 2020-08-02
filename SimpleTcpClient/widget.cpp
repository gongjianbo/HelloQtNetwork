#include "widget.h"
#include "ui_widget.h"

#include <QHostAddress>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Client");

    initClient();
}

Widget::~Widget()
{
    //析构关闭连接
    //client->disconnectFromHost();
    //if(client->state()!=QAbstractSocket::UnconnectedState){
    //    client->waitForDisconnected();
    //}
    //关闭套接字的I/O设备，并调用disconnectFromHost()关闭套接字的连接。
    //client->close();
    //中止当前连接并重置套接字。与disconnectFromHost()不同，
    //此函数立即关闭套接字，并丢弃写缓冲区中的所有待处理数据。
    client->abort();
    delete ui;
}

void Widget::initClient()
{
    //创建client对象
    client = new QTcpSocket(this);

    //点击连接，根据ui设置的服务器地址进行连接
    connect(ui->btnConnect,&QPushButton::clicked,[this]{
        //判断当前是否已连接，连接了就断开
        if(client->state()==QAbstractSocket::ConnectedState){
            //如果使用disconnectFromHost()不会重置套接字，isValid还是会为true
            client->abort();
        }else if(client->state()==QAbstractSocket::UnconnectedState){
            //从界面上读取ip和端口
            const QHostAddress address=QHostAddress(ui->editAddress->text());
            const unsigned short port=ui->editPort->text().toUShort();
            //连接服务器
            client->connectToHost(address,port);
        }else{
            ui->textRecv->append("It is not ConnectedState or UnconnectedState");
        }
    });

    //连接状态
    connect(client,&QTcpSocket::connected,[this]{
        //已连接就设置为不可编辑
        ui->btnConnect->setText("Disconnect");
        ui->editAddress->setEnabled(false);
        ui->editPort->setEnabled(false);
        updateState();
    });
    connect(client,&QTcpSocket::disconnected,[this]{
        //断开连接还原状态
        ui->btnConnect->setText("Connect");
        ui->editAddress->setEnabled(true);
        ui->editPort->setEnabled(true);
        updateState();
    });

    //发送数据
    connect(ui->btnSend,&QPushButton::clicked,[this]{
        //判断是可操作，isValid表示准备好读写
        if(!client->isValid())
            return;
        //将发送区文本发送给客户端
        const QByteArray send_data=ui->textSend->toPlainText().toUtf8();
        //数据为空就返回
        if(send_data.isEmpty())
            return;
        client->write(send_data);
        //client->waitForBytesWritten();
    });

    //收到数据，触发readyRead
    connect(client,&QTcpSocket::readyRead,[this]{
        //没有可读的数据就返回
        if(client->bytesAvailable()<=0)
            return;
        //注意收发两端文本要使用对应的编解码
        const QString recv_text=QString::fromUtf8(client->readAll());
        ui->textRecv->append(QString("[%1:%2]")
                             .arg(client->peerAddress().toString())
                             .arg(client->peerPort()));
        ui->textRecv->append(recv_text);
    });

    //错误信息
    connect(client,&QTcpSocket::errorOccurred,[this](QAbstractSocket::SocketError){
        ui->textRecv->append("Socket Error:"+client->errorString());
    });
}

void Widget::updateState()
{
    //将当前client地址和端口写在标题栏
    if(client->state()==QAbstractSocket::ConnectedState){
        setWindowTitle(QString("Client[%1:%2]")
                       .arg(client->localAddress().toString())
                       .arg(client->localPort()));
    }else{
        setWindowTitle("Client");
    }
}

