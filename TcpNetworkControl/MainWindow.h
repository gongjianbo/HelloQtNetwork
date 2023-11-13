#pragma once
#include <QMainWindow>
#include <QThread>
#include "ControlClient.h"
#include "ControlServer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 主界面
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // 连接
    void doConnect();
    // 断开
    void doDisconnect();
    // 心跳
    void doKeepAlive();
    // 代理端信息
    void doPolicyInfo();
    // 设备信息
    void doDeviceInfo();
    // 触发模式设置
    void doTriggerMode();
    // 软触发
    void doSoftTrigger();
    // 积分时间设置
    void doIntegTime();

    // 模拟服务端通知有图像
    void newImage();
    // 模拟客户端下载图像
    void loadImage(const QString &deviceId, const QString &imageId);

private:
    Ui::MainWindow *ui;

    // 服务端
    ControlServer *ctrlServer{nullptr};
    ControlServer *imageServer{nullptr};
    // 设备控制
    ControlClient *ctrlClient{nullptr};
    QThread *clientThread{nullptr};
};
