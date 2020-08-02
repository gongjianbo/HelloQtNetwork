#include "widget.h"
#include "ui_widget.h"

#include <QHostAddress>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Server");

    initServer();
}

Widget::~Widget()
{
    //关闭server
    closeServer();
    delete ui;
}

void Widget::initServer()
{
    //创建Server对象
    server = new QTcpServer(this);

    //点击监听按钮，开始监听
    connect(ui->btnListen,&QPushButton::clicked,[this]{
        //判断当前是否已开启，是则close，否则listen
        if(server->isListening()){
            //server->close();
            closeServer();
            //关闭server后恢复界面状态
            ui->btnListen->setText("Listen");
            ui->editAddress->setEnabled(true);
            ui->editPort->setEnabled(true);
        }else{
            //从界面上读取ip和端口
            //可以使用 QHostAddress::Any 监听所有地址的对应端口
            const QString address_text=ui->editAddress->text();
            const QHostAddress address=(address_text=="Any")
                    ?QHostAddress::Any
                   :QHostAddress(address_text);
            const unsigned short port=ui->editPort->text().toUShort();
            //开始监听，并判断是否成功
            if(server->listen(address,port)){
                //连接成功就修改界面按钮提示，以及地址栏不可编辑
                ui->btnListen->setText("Close");
                ui->editAddress->setEnabled(false);
                ui->editPort->setEnabled(false);
            }
        }
        updateState();
    });

    //监听到新的客户端连接请求
    connect(server,&QTcpServer::newConnection,this,[this]{
        //如果有新的连接就取出
        while(server->hasPendingConnections())
        {
            //nextPendingConnection返回下一个挂起的连接作为已连接的QTcpSocket对象
            //套接字是作为服务器的子级创建的，这意味着销毁QTcpServer对象时会自动删除该套接字。
            //最好在完成处理后显式删除该对象，以避免浪费内存。
            //返回的QTcpSocket对象不能从另一个线程使用，如有需要可重写incomingConnection().
            QTcpSocket *socket=server->nextPendingConnection();
            clientList.append(socket);
            ui->textRecv->append(QString("[%1:%2] Soket Connected")
                                 .arg(socket->peerAddress().toString())
                                 .arg(socket->peerPort()));

            //关联相关操作的信号槽
            //收到数据，触发readyRead
            connect(socket,&QTcpSocket::readyRead,[this,socket]{
                //没有可读的数据就返回
                if(socket->bytesAvailable()<=0)
                    return;
                //注意收发两端文本要使用对应的编解码
                const QString recv_text=QString::fromUtf8(socket->readAll());
                ui->textRecv->append(QString("[%1:%2]")
                                     .arg(socket->peerAddress().toString())
                                     .arg(socket->peerPort()));
                ui->textRecv->append(recv_text);
            });

            //错误信息
            connect(socket,&QTcpSocket::errorOccurred,[this,socket](QAbstractSocket::SocketError){
                ui->textRecv->append(QString("[%1:%2] Soket Error:%3")
                                     .arg(socket->peerAddress().toString())
                                     .arg(socket->peerPort())
                                     .arg(socket->errorString()));
            });

            //连接断开，销毁socket对象，这是为了开关server时socket正确释放
            connect(socket,&QTcpSocket::disconnected,[this,socket]{
                socket->deleteLater();
                clientList.removeOne(socket);
                ui->textRecv->append(QString("[%1:%2] Soket Disonnected")
                                     .arg(socket->peerAddress().toString())
                                     .arg(socket->peerPort()));
                updateState();
            });
        }
        updateState();
    });

    //server向client发送内容
    connect(ui->btnSend,&QPushButton::clicked,[this]{
        //判断是否开启了server
        if(!server->isListening())
            return;
        //将发送区文本发送给客户端
        const QByteArray send_data=ui->textSend->toPlainText().toUtf8();
        //数据为空就返回
        if(send_data.isEmpty())
            return;
        for(QTcpSocket *socket:clientList)
        {
            socket->write(send_data);
            //socket->waitForBytesWritten();
        }
    });

    //server的错误信息
    //如果发生错误，则serverError()返回错误的类型，
    //并且可以调用errorString()以获取对所发生事件的易于理解的描述
    connect(server,&QTcpServer::acceptError,[this](QAbstractSocket::SocketError){
        ui->textRecv->append("Server Error:"+server->errorString());
    });
}

void Widget::closeServer()
{
    //停止服务
    server->close();
    for(QTcpSocket * socket:clientList)
    {
        //断开与客户端的连接
        socket->disconnectFromHost();
        if(socket->state()!=QAbstractSocket::UnconnectedState){
            socket->abort();
        }
    }
}

void Widget::updateState()
{
    //将当前server地址和端口、客户端连接数写在标题栏
    if(server->isListening()){
        setWindowTitle(QString("Server[%1:%2] connections:%3")
                       .arg(server->serverAddress().toString())
                       .arg(server->serverPort())
                       .arg(clientList.count()));
    }else{
        setWindowTitle("Server");
    }
}

