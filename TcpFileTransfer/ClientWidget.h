#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QThread>

#include "ClientOperate.h"

namespace Ui {
class ClientWidget;
}

//客户端界面--客户端作为发送
class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget();

signals:
    //使用信号槽操作线程中的ClientOperate
    void connectTcp(const QString &address,quint16 port);
    void disconnectTcp();

private:
    Ui::ClientWidget *ui;

    //线程
    QThread *thread;
    //client处理放到线程中
    ClientOperate *operate;
};

#endif // CLIENTWIDGET_H
