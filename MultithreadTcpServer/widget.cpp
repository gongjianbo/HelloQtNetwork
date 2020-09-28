#include "widget.h"
#include "ui_widget.h"

#include <QtWidgets>
#include <QHostAddress>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    initClient();
    initServer();
}

Widget::~Widget()
{
    delete ui;
    for(QTcpSocket* socket:clientList)
        socket->abort();
    qDeleteAll(clientList);
}

void Widget::initClient()
{
    //表格里随便弄几个客户端socket列表
    //每个客户端可以给server发文本数据
    const int row_count=10;
    ui->clientTable->setRowCount(row_count);
    for(int row=0;row<row_count;row++)
    {
        //socket
        QTcpSocket *socket=new QTcpSocket;
        clientList.push_back(socket);
        //col端口
        QLabel *lab=new QLabel;
        ui->clientTable->setCellWidget(row,0,lab);
        //col操作
        QPushButton *btn_connect=new QPushButton("connect");
        connect(btn_connect,&QPushButton::clicked,this,[=]{
            if(socket->state()==QAbstractSocket::ConnectedState){
                socket->abort();
            }else{
                socket->connectToHost(QHostAddress("127.0.0.1"),23456);
            }
        });
        connect(socket,&QTcpSocket::connected,this,[=]{
            lab->setText(QString::number(socket->localPort()));
            btn_connect->setText("disconnect");
        });
        connect(socket,&QTcpSocket::disconnected,this,[=]{
            lab->clear();
            btn_connect->setText("connect");
        });
        ui->clientTable->setCellWidget(row,1,btn_connect);
        //col发送
        QPushButton *btn_send=new QPushButton("send");
        ui->clientTable->setCellWidget(row,2,btn_send);
        //col文本框
        QLineEdit *edit=new QLineEdit;
        edit->setText(QString("hello %1").arg(row));
        ui->clientTable->setCellWidget(row,3,edit);
        connect(btn_send,&QPushButton::clicked,this,[=]{
            if(socket->isValid())
                socket->write(edit->text().toLatin1());
        });
    }
}

void Widget::initServer()
{
    server=new ServerOperate(this);
    //新的连接
    connect(server,&ServerOperate::clientConnected,
            this,[=](quint16 port,const QString &id){
        const int row_count=ui->serverTable->rowCount();
        QTableWidget *table=ui->serverTable;
        table->insertRow(row_count);
        table->setCellWidget(row_count,0,new QLabel(QString::number(port)));
        table->setCellWidget(row_count,1,new QLabel(id));
        QPushButton *btn=new QPushButton("close");
        connect(btn,&QPushButton::clicked,this,[=]{
            server->closeConnect(port);
        });
        table->setCellWidget(row_count,2,btn);
        table->setCellWidget(row_count,3,new QLabel);
    });
    //连接断开
    //暂时用端口号来遍历查找对应的连接
    connect(server,&ServerOperate::clientDisconnected,
            this,[=](quint16 port){
        const int row_count=ui->serverTable->rowCount();
        const QString port_str=QString::number(port);
        for(int row=0;row<row_count;row++)
        {
            QLabel *labport=qobject_cast<QLabel*>(ui->serverTable->cellWidget(row,0));
            if(labport&&labport->text()==port_str)
            {
                ui->serverTable->removeRow(row);
                return;
            }
        }
    });
    //客户端发送过来的消息
    //暂时用端口号来遍历查找对应的连接
    connect(server,&ServerOperate::clientMessage,
            this,[=](quint16 port,const QString &msg){
        const int row_count=ui->serverTable->rowCount();
        const QString port_str=QString::number(port);
        for(int row=0;row<row_count;row++)
        {
            QLabel *labport=qobject_cast<QLabel*>(ui->serverTable->cellWidget(row,0));
            if(labport&&labport->text()==port_str)
            {
                QLabel *labmsg=qobject_cast<QLabel*>(ui->serverTable->cellWidget(row,3));
                if(labmsg){
                    labmsg->setText(msg);
                }
                return;
            }
        }
    });
}
