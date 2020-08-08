#include "ServerWidget.h"
#include "ui_ServerWidget.h"

#include <QFileDialog>

ServerWidget::ServerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerWidget)
{
    ui->setupUi(this);

    thread = new QThread(this);
    operate = new ServerOperate;
    operate->moveToThread(thread);
    //退出时释放
    connect(thread,&QThread::finished,operate,&ServerOperate::deleteLater);
    //点击了listen，服务端监听开关
    connect(ui->btnListen,&QPushButton::clicked,[this]{
        if(operate->isListening()){
            emit dislisten();
        }else{
            emit listen(ui->editAddress->text(),ui->editPort->text().toUShort());
        }
    });
    connect(this,&ServerWidget::listen,operate,&ServerOperate::listen);
    connect(this,&ServerWidget::dislisten,operate,&ServerOperate::dislisten);
    //服务器监听状态改变
    connect(operate,&ServerOperate::listenStateChanged,this,[this](bool isListen){
        ui->btnListen->setText(isListen?"Dislisten":"Listen");
        ui->editAddress->setEnabled(!isListen);
        ui->editPort->setEnabled(!isListen);
        ui->editPath->setEnabled(!isListen);
        ui->btnSelect->setEnabled(!isListen);
    });
    //选择文件路径
    connect(ui->btnSelect,&QPushButton::clicked,[=]{
        const QString dir_path=QFileDialog::getExistingDirectory(this);
        ui->editPath->setText(dir_path);
    });
    connect(ui->editPath,&QLineEdit::textChanged,operate,&ServerOperate::setFilePath);
    //取消文件传输
    connect(ui->btnCancel,&QPushButton::clicked,operate,&ServerOperate::cancelFileTransfer);
    //日志
    connect(operate,&ServerOperate::logMessage,this,[=](const QString &msg){
        ui->textEdit->append(msg);
    });
    //进度条
    connect(operate,&ServerOperate::progressChanged,ui->progressBar,&QProgressBar::setValue);

    //启动线程
    thread->start();
}

ServerWidget::~ServerWidget()
{
    thread->quit();
    thread->wait();
    delete ui;
}

