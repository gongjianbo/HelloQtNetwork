#include "ControlServer.h"
#include <QFile>
#include <QDebug>

ControlServer::ControlServer(QObject *parent)
    : MyTcpServer(parent)
{

}

ControlServer::~ControlServer()
{

}

void ControlServer::onRecvFrame(MyTcpSocket *socket, const ProtocolFrame &frame)
{
    qDebug() << "server recv frame" << frame.command << frame.content << frame.error;
    // 这里需要和设备管理类交互，进行控制
    if (frame.command == CC_PolicyInfo) {
        // 获取代理端信息
        socket->sendFrame(getPolicyInfo());
    } else if (frame.command == CC_DeviceInfo) {
        // 获取设备信息
        socket->sendFrame(getDeviceInfo(frame.content.value("deviceId").toString()));
    } else if (frame.command == CC_TriggerMode) {
        // 设置触发模式
        setTriggerMode(frame.content.value("deviceId").toString(),
                       frame.content.value("triggerMode").toInt());
    } else if (frame.command == CC_SoftTrigger) {
        // 软触发
        socket->sendFrame(softTrigger(frame.content.value("deviceId").toString()));
    } else if (frame.command == CC_IntegTime) {
        // 设置积分时间
        setIntegTime(frame.content.value("deviceId").toString(),
                     frame.content.value("integrationTime").toInt());
    } else if (frame.command == CC_LoadImage) {
        // 获取图片
        socket->sendFrame(getImage(frame.content.value("deviceId").toString(),
                                   frame.content.value("imageId").toString()));
        // 等待发送结束
        socket->waitWritten();
        // 断开连接
        socket->disconnectSocket();
    }
}

ProtocolFrame ControlServer::getPolicyInfo()
{
    // DeviceManager::getDeviceList();
    ProtocolFrame frame;
    frame.command = CC_PolicyInfo;
    frame.error = CE_NoErr;
    frame.content = QJsonObject{
        {"ip", "xx.xx.xx.xx"},
        {"version", "v0.0.0(2023-11-11)"},
        {"startTime", "2023-11-12 xxx"},
        {"deviceList", QJsonValue(QJsonArray{"11110001", "11110002"})}
    };
    return frame;
}

ProtocolFrame ControlServer::getDeviceInfo(const QString &deviceId)
{
    // DeviceManager::getDeviceInfo();
    ProtocolFrame frame;
    frame.command = CC_DeviceInfo;
    frame.error = deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"deviceType", 0},
        {"status", 3},
        {"width", 200},
        {"height", 200},
        {"integrationTime", 100},
        {"triggerMode", 0}
    };
    return frame;
}

void ControlServer::syncStatus(const QString &deviceId, int status, int error)
{
    // DeviceManager::emit deviceStatusChanged(deviceId, status, error);
    ProtocolFrame frame;
    frame.command = CC_StatusSync;
    // 根据操作结果设置错误码
    frame.error = (status == CS_DeviceError) ? error : (deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr);
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"status", status}
    };
    emit syncAll(frame);
}

void ControlServer::syncImage(const QString &deviceId, const QString &imageId)
{
    // DeviceManager::emit imageLoadFinished(deviceId, imageId);
    ProtocolFrame frame;
    frame.command = CC_StatusSync;
    // 根据操作结果设置错误码
    frame.error = deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"status", (int)CS_TransFinish},
        {"imageId", imageId}
    };
    emit syncAll(frame);
}

void ControlServer::setTriggerMode(const QString &deviceId, unsigned char triggerMode)
{
    // 设置类的接口
    // 1.可以server发信号给device设置后，device发信号反馈到server槽函数，server再同步给所有的客户端
    // 2.device接口加锁在server中直接操作
    ProtocolFrame frame;
    frame.command = CC_TriggerMode;
    // 根据操作结果设置错误码
    frame.error = deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"triggerMode", (int)triggerMode}
    };
    emit syncAll(frame);
}

ProtocolFrame ControlServer::softTrigger(const QString &deviceId)
{
    ProtocolFrame frame;
    frame.command = CC_SoftTrigger;
    // 根据操作结果设置错误码
    frame.error = deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr;
    frame.content = QJsonObject{{"deviceId", deviceId}};
    return frame;
}

void ControlServer::setIntegTime(const QString &deviceId, unsigned short integTime)
{
    ProtocolFrame frame;
    frame.command = CC_IntegTime;
    // 根据操作结果设置错误码
    frame.error = deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"integrationTime", (int)integTime}
    };

    emit syncAll(frame);
}

ProtocolFrame ControlServer::getImage(const QString &deviceId, const QString &imageId)
{
    ProtocolFrame frame;
    frame.command = CC_LoadImage;
    // 根据操作结果设置错误码
    frame.error = deviceId.isEmpty() ? CE_ImageIdErr : CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"imageId", imageId},
        {"width", 200},
        {"height", 200}
    };
    // 模拟数据，注意 32 位不要一次性加载大文件
    QFile file("D:/Download/qt-unified-windows-x64-4.6.0-online.exe");
    if (file.open(QIODevice::ReadOnly)) {
        frame.data = file.readAll();
        file.close();
    } else {
        QByteArray data(200 * 200 * 2, 0x00);
        data[0] = (char)0xFF;
        data[data.size() - 1] = (char)0xFF;
        frame.data = data;
    }
    return frame;
}
