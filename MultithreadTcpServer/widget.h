#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include "ServerOperate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

//为了便于演示，我把server和client的相关操作都卸载了同一个界面
//为了简化流程，地址我都是随机或者固定值，主要目的是展示tcpserver多线程的运用
class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void initClient();
    void initServer();

private:
    Ui::Widget *ui;

    //模拟的客户端列表，在ui线程直接操作
    QList<QTcpSocket *> clientList;
    //服务端操作，简易的封装
    ServerOperate *server;
};
#endif // WIDGET_H
