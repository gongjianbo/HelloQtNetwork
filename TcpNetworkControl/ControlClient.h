#pragma once
#include "MyTcpSocket.h"

/**
 * @brief 设备控制客户端
 * @author 龚建波
 * @date 2023-11-01
 */
class ControlClient : public MyTcpSocket
{
    Q_OBJECT
public:
    explicit ControlClient();
    ~ControlClient();

    // 获取代理端信息
    void getPolicyInfo();
    // 获取设备信息
    void getDeviceInfo(const QString &deviceId);
    // 设置拍图模式，0-AED模式，1=软触发
    void setTriggerMode(const QString &deviceId, unsigned char triggerMode);
    // 执行软触发
    void softTrigger(const QString &deviceId);
    // 设置积分时间，单位毫秒
    void setIntegTime(const QString &deviceId, unsigned short integTime);

    // 请求图像
    void requestImage(const QString &deviceId, const QString &imageId);
};
