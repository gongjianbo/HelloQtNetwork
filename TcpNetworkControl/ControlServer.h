#pragma once
#include "MyTcpServer.h"

/**
 * @brief 设备控制服务端
 * @author 龚建波
 * @date 2023-11-01
 * @details
 * ControlServer 通过信号槽或其他中介者和设备 IO 操作的对象交互
 */
class ControlServer : public MyTcpServer
{
    Q_OBJECT
public:
    explicit ControlServer(QObject *parent = nullptr);
    ~ControlServer();

    // 客户端发来的命令，会在线程中回调该接口
    void onRecvFrame(MyTcpSocket *socket, const ProtocolFrame &frame) override;

    // 获取代理端信息
    ProtocolFrame getPolicyInfo();
    // 获取设备信息
    ProtocolFrame getDeviceInfo(const QString &deviceId);
    // 同步设备状态
    void syncStatus(const QString &deviceId, int status, int error);
    // 同步图像加载完成状态
    void syncImage(const QString &deviceId, const QString &imageId);

    // 设置拍图模式，0-AED模式，1=软触发
    void setTriggerMode(const QString &deviceId, unsigned char triggerMode);
    // 执行软触发
    ProtocolFrame softTrigger(const QString &deviceId);
    // 设置积分时间，单位毫秒
    void setIntegTime(const QString &deviceId, unsigned short integTime);

    // 获取图片
    ProtocolFrame getImage(const QString &deviceId, const QString &imageId);
};
