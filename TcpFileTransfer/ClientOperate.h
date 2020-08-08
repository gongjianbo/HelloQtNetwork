#ifndef CLIENTOPERATE_H
#define CLIENTOPERATE_H

#include <QObject>

//客户端socket处理--客户端作为发送
class ClientOperate : public QObject
{
    Q_OBJECT
public:
    explicit ClientOperate(QObject *parent = nullptr);

signals:

};

#endif // CLIENTOPERATE_H
