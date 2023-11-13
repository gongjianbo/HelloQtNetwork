#pragma once
#include <QObject>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QtEndian>

// 命令类型
enum ControlCommandType {
    // 心跳保活
    CC_KeepAlive = 0,
    // 代理端信息
    CC_PolicyInfo = 1,
    // 设备信息
    CC_DeviceInfo = 2,
    // 设备状态同步
    CC_StatusSync = 3,

    // 触发模式
    CC_TriggerMode = 100,
    // 软触发
    CC_SoftTrigger = 101,
    // 积分时间
    CC_IntegTime = 102,

    // 加载图片
    CC_LoadImage = 200
};

// 设备状态
enum ControlStatusCode
{
    // 有设备连接
    CS_Connect = 0,
    // 设备断开
    CS_Disonnect = 1,
    // 等待拍图
    CS_WaitTrigger = 2,
    // 正在从设备Load图像
    CS_TransImage = 3,
    // 图片Load完成等待发送给客户端
    CS_TransFinish = 4,
    // 设备异常
    CS_DeviceError = 5
};

// 错误码
enum ControlErrorCode
{
    // 无错误
    CE_NoErr = 0,
    // 无效的命令码
    CE_CommandErr = 0,
    // 无效的设备Id
    CE_DeviceIdErr = 0,
    // 无效的图像Id
    CE_ImageIdErr = 0,
    // 其他错误
    CE_OtherErr
};

// 帧信息
struct ProtocolFrame
{
    // 命令类型 CP_CommandType
    int command;
    // 错误码 CP_ErrorCode
    int error;
    // 控制消息
    QJsonObject content;
    // 数据内容
    QByteArray data;
};

/**
 * @brief 协议解析
 * @author 龚建波
 * @date 2023-10-31
 */
class ProtocolParser
{
public:
    ProtocolParser();
    ~ProtocolParser();

    /**
     * @brief 帧内容根据协议装箱为字节数据
     * @param frame 帧内容
     * @param ok 可传入 bool 接收返回
     * @return 返回按协议生成的数据
     */
    static QByteArray pack(const ProtocolFrame &frame, bool *ok);

    /**
     * @brief 字节数据根据协议拆箱为数据结构
     * @param data 帧字节数据
     * @param ok 可传入 bool 接收返回
     * @return 返回按协议生成的数据
     */
    static ProtocolFrame unpack(const QByteArray &data, bool *ok);

    // uin32 转小端字节数组
    static QByteArray uint32ToBytes(unsigned int num);
    // 小端字节数组转 uint32
    static unsigned int uint32FromBytes(QByteArray bytes);

public:
    static constexpr char FRAME_HEAD[4] = { (char)0xFF, (char)0xFF, 0x00, 0x00 };
    static constexpr char FRAME_TAIL[4] = { 0x00, 0x00, (char)0xFF, (char)0xFF };
    static constexpr char FRAME_EMPTY[4] = { 0x00, 0x00, 0x00, 0x00 };
};

