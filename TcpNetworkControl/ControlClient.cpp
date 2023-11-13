#include "ControlClient.h"
#include <memory.h>
#include <QHostAddress>
#include <QDebug>

ControlClient::ControlClient()
{

}

ControlClient::~ControlClient()
{

}

void ControlClient::getPolicyInfo()
{
    ProtocolFrame frame;
    frame.command = CC_PolicyInfo;
    frame.error = CE_NoErr;
    sendFrame(frame);
}

void ControlClient::getDeviceInfo(const QString &deviceId)
{
    ProtocolFrame frame;
    frame.command = CC_DeviceInfo;
    frame.error = CE_NoErr;
    frame.content = QJsonObject{{"deviceId", deviceId}};
    sendFrame(frame);
}

void ControlClient::setTriggerMode(const QString &deviceId, unsigned char triggerMode)
{
    ProtocolFrame frame;
    frame.command = CC_TriggerMode;
    frame.error = CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"triggerMode", (int)triggerMode}
    };
    sendFrame(frame);
}

void ControlClient::softTrigger(const QString &deviceId)
{
    ProtocolFrame frame;
    frame.command = CC_SoftTrigger;
    frame.error = CE_NoErr;
    frame.content = QJsonObject{{"deviceId", deviceId}};
    sendFrame(frame);
}

void ControlClient::setIntegTime(const QString &deviceId, unsigned short integTime)
{
    ProtocolFrame frame;
    frame.command = CC_IntegTime;
    frame.error = CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"integrationTime", (int)integTime}
    };
    sendFrame(frame);
}

void ControlClient::requestImage(const QString &deviceId, const QString &imageId)
{
    ProtocolFrame frame;
    frame.command = CC_LoadImage;
    frame.error = CE_NoErr;
    frame.content = QJsonObject{
        {"deviceId", deviceId},
        {"imageId", imageId}
    };
    sendFrame(frame);
}
