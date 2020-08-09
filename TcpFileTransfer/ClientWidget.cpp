#include "ClientWidget.h"
#include "ui_ClientWidget.h"

#include <QFileDialog>

ClientWidget::ClientWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientWidget)
{
    ui->setupUi(this);

    thread = new QThread(this);
    operate = new ClientOperate;
    operate->moveToThread(thread);
    //退出时释放
    connect(thread,&QThread::finished,operate,&ClientOperate::deleteLater);
    //点击了connect
    connect(ui->btnConnect,&QPushButton::clicked,[this]{
        if(operate->isConnected()){
            emit disconnectTcp();
        }else{
            emit connectTcp(ui->editAddress->text(),ui->editPort->text().toUShort());
        }
    });
    connect(this,&ClientWidget::connectTcp,operate,&ClientOperate::connectTcp);
    connect(this,&ClientWidget::disconnectTcp,operate,&ClientOperate::disconnectTcp);
    //服务器监听状态改变
    connect(operate,&ClientOperate::connectStateChanged,this,[this](bool isConnected){
        ui->btnConnect->setText(isConnected?"Disconnect":"Connect");
        ui->editAddress->setEnabled(!isConnected);
        ui->editPort->setEnabled(!isConnected);
    });
    //选择文件路径
    connect(ui->btnSelect,&QPushButton::clicked,[=]{
        const QString dir_path=QFileDialog::getOpenFileName(this);
        ui->editPath->setText(dir_path);
    });
    connect(ui->editPath,&QLineEdit::textChanged,operate,&ClientOperate::setFilePath);
    //文件传输
    connect(ui->btnSend,&QPushButton::clicked,operate,&ClientOperate::startFileTransfer);
    connect(ui->btnCancel,&QPushButton::clicked,operate,&ClientOperate::cancelFileTransfer);
    //日志
    connect(operate,&ClientOperate::logMessage,this,[=](const QString &msg){
        ui->textEdit->append(msg);
    });
    //进度条
    connect(operate,&ClientOperate::progressChanged,ui->progressBar,&QProgressBar::setValue);

    //启动线程
    thread->start();
}

ClientWidget::~ClientWidget()
{
    thread->quit();
    thread->wait();
    delete ui;
}
