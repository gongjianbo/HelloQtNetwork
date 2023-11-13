#include "ProtocolParser.h"
#include <QJsonDocument>

ProtocolParser::ProtocolParser()
{

}

ProtocolParser::~ProtocolParser()
{

}

QByteArray ProtocolParser::pack(const ProtocolFrame &frame, bool *ok)
{
    if (ok) {
        *ok = false;
    }
    QByteArray data;
    // Control 信息
    QJsonObject control_json;
    control_json.insert("command", frame.command);
    control_json.insert("content", frame.content);
    control_json.insert("error", frame.error);
    QByteArray control_data = QJsonDocument(control_json).toJson(QJsonDocument::Compact);
    QByteArray control_len = uint32ToBytes(control_data.size());
    data.append(control_len);
    data.append(control_data);
    // Data 信息
    QByteArray data_len = uint32ToBytes(frame.data.size());
    data.append(data_len);
    if (frame.data.size() > 0) {
        data.append(frame.data);
    }
    if (ok) {
        *ok = true;
    }
    return data;
}

ProtocolFrame ProtocolParser::unpack(const QByteArray &data, bool *ok)
{
    if (ok) {
        *ok = false;
    }
    ProtocolFrame frame;
    unsigned int offset = 0;
    // Control 长度 + Data 长度 = 8 字节
    if (data.size() - offset < 8) {
        return frame;
    }
    unsigned int control_len = uint32FromBytes(data.mid(offset, 4));
    offset += 4;
    // Control 数据 + Data 长度
    if (data.size() - offset < control_len + 4) {
        return frame;
    }
    QByteArray control_data = data.mid(offset, control_len);
    offset += control_len;
    QJsonDocument control_doc = QJsonDocument::fromJson(control_data);
    if (!control_doc.isObject()) {
        return frame;
    }
    QJsonObject control_json = control_doc.object();
    if (!control_json.contains("command")) {
        return frame;
    }
    frame.command = control_json.value("command").toInt();
    frame.content = control_json.value("content").toObject();
    frame.error = control_json.value("eror").toInt();
    unsigned int data_len = uint32FromBytes(data.mid(offset, 4));
    offset += 4;
    // Data 数据
    if (data.size() - offset < data_len) {
        return frame;
    }
    frame.data = data.mid(offset, data_len);
    offset += data_len;
    if (offset != data.size()) {
        return frame;
    }
    if (ok) {
        *ok = true;
    }
    return frame;
}

QByteArray ProtocolParser::uint32ToBytes(unsigned int num)
{
    QByteArray bytes(4, 0x00);
    qToLittleEndian<unsigned int>(num, bytes.data());
    return bytes;
}

unsigned int ProtocolParser::uint32FromBytes(QByteArray bytes)
{
    if (bytes.size() != sizeof(unsigned int))
        return 0;
    unsigned int num;
    qFromLittleEndian<unsigned int>(bytes.data(), 1, &num);
    return num;
}
