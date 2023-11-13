#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <thread>
#include <QApplication>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ctrlServer = new ControlServer(this);
    ctrlServer->listen(QHostAddress("127.0.0.1"), 12315);

    imageServer = new ControlServer(this);
    imageServer->listen(QHostAddress("127.0.0.1"), 12316);

    ctrlClient = new ControlClient();
    clientThread = new QThread(this);
    ctrlClient->moveToThread(clientThread);
    connect(clientThread, &QThread::started, ctrlClient, &ControlClient::init);
    connect(clientThread, &QThread::finished, ctrlClient, &ControlClient::deleteLater);
    ui->btnDisconnect->setEnabled(false);
    connect(ctrlClient, &ControlClient::connectStateChanged, this, [this](bool connected){
        ui->btnConnect->setEnabled(!connected);
        ui->btnDisconnect->setEnabled(connected);
        // 设备连接后，先获取代理端信息&&设备列表
        QMetaObject::invokeMethod(ctrlClient, [this](){
            ctrlClient->getPolicyInfo();
        });
    });
    qRegisterMetaType<ProtocolFrame>("ProtocolFrame");
    connect(ctrlClient, &ControlClient::recvFrame, this, [this](const ProtocolFrame &frame){
        qDebug() << "client recv frame" << frame.command << frame.content << frame.error;
        if (frame.error != CE_NoErr)
            return;
        if (frame.command == CC_PolicyInfo) {
            // 更新代理端信息
            QJsonArray arr = frame.content.value("deviceList").toArray();
            QStringList ids;
            for (int i = 0; i < arr.size(); i++)
            {
                ids.append(arr[i].toString());
            }
            ui->deviceListBox->clear();
            ui->deviceListBox->addItems(ids);
        } else if (frame.command == CC_DeviceInfo) {
            // 设备信息
        } else if (frame.command == CC_StatusSync) {
            // 设备状态
            QString device_id = frame.content.value("deviceId").toString();
            int status = frame.content.value("status").toInt();
            if (status == CS_TransFinish && device_id == ui->deviceListBox->currentText()) {
                QString image_id = frame.content.value("imageId").toString();
                // 来图了，开启线程去下载图片
                loadImage(device_id, image_id);
            }
        }
    });
    clientThread->start();

    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::doConnect);
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::doDisconnect);
    connect(ui->btnKeepAlive, &QPushButton::clicked, this, &MainWindow::doKeepAlive);
    connect(ui->btnPolicyInfo, &QPushButton::clicked, this, &MainWindow::doPolicyInfo);
    connect(ui->btnDeviceInfo, &QPushButton::clicked, this, &MainWindow::doDeviceInfo);
    connect(ui->btnTriggerMode, &QPushButton::clicked, this, &MainWindow::doTriggerMode);
    connect(ui->btnSoftTrigger, &QPushButton::clicked, this, &MainWindow::doSoftTrigger);
    connect(ui->btnIntegTime, &QPushButton::clicked, this, &MainWindow::doIntegTime);

    connect(ui->btnImage, &QPushButton::clicked, this, &MainWindow::newImage);
}

MainWindow::~MainWindow()
{
    delete ui;
    clientThread->quit();
    clientThread->wait();
}

void MainWindow::doConnect()
{
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->connectServer("127.0.0.1", 12315);
    });
}

void MainWindow::doDisconnect()
{
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->disconnectSocket();
    });
}

void MainWindow::doKeepAlive()
{
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->keepAlive();
    });
}

void MainWindow::doPolicyInfo()
{
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->getPolicyInfo();
    });
}

void MainWindow::doDeviceInfo()
{
    if (ui->deviceListBox->count() == 0)
        return;
    const QString device_id = ui->deviceListBox->currentText();
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->getDeviceInfo(device_id);
    });
}

void MainWindow::doTriggerMode()
{
    if (ui->deviceListBox->count() == 0)
        return;
    const QString device_id = ui->deviceListBox->currentText();
    const int trigger_mode = ui->triggerModeBox->currentIndex();
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->setTriggerMode(device_id, trigger_mode);
    });
}

void MainWindow::doSoftTrigger()
{
    if (ui->deviceListBox->count() == 0)
        return;
    const QString device_id = ui->deviceListBox->currentText();
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->softTrigger(device_id);
    });
}

void MainWindow::doIntegTime()
{
    if (ui->deviceListBox->count() == 0)
        return;
    const QString device_id = ui->deviceListBox->currentText();
    const unsigned short integ_time = ui->integTimeBox->value();
    QMetaObject::invokeMethod(ctrlClient, [=](){
        ctrlClient->setIntegTime(device_id, integ_time);
    });
}

void MainWindow::newImage()
{
    if (ui->deviceListBox->count() == 0)
        return;
    ctrlServer->syncImage(ui->deviceListBox->currentText(), "idxxx");
}

void MainWindow::loadImage(const QString &deviceId, const QString &imageId)
{
    std::thread th([=]{
        QEventLoop loop;
        ControlClient socket;
        socket.init();
        // 传输时间较长，不要心跳检测
        socket.connectServer("127.0.0.1", 12316, false);
        if (!socket.isConnected())
            return;
        socket.requestImage(deviceId, imageId);
        connect(&socket, &ControlClient::connectStateChanged, [&](bool connected){
            if (!connected) {
                // 异常断开
                loop.quit();
            }
        });
        connect(&socket, &ControlClient::recvFrame, [&](const ProtocolFrame &frame){
            qDebug() << "client recv file" << frame.command << frame.content << frame.error;
            if (frame.command == CC_LoadImage) {
                // 图像数据
                if (frame.error == CE_NoErr) {
                    // 保存图片
                    QFile file(qApp->applicationDirPath() + "/image.raw");
                    if (file.open(QIODevice::WriteOnly)) {
                        qDebug() << "save file" << frame.data.size() << file.fileName();
                        file.write(frame.data);
                        file.close();
                    }
                }
                loop.quit();
            }
        });
        // 等待图片传完或者网络断开
        loop.exec();
        socket.disconnectSocket();
        qDebug() << "load image finished";
    });
    th.detach();
}
