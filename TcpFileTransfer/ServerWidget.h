#ifndef SERVERWIDGET_H
#define SERVERWIDGET_H

#include <QWidget>
#include <QThread>

#include "ServerOperate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWidget; }
QT_END_NAMESPACE

//服务端界面--服务端作为接收
class ServerWidget : public QWidget
{
    Q_OBJECT

public:
    ServerWidget(QWidget *parent = nullptr);
    ~ServerWidget();

signals:
    //使用信号槽操作线程中的ServerOperate
    void listen(const QString &address,quint16 port);
    void dislisten();

private:
    Ui::ServerWidget *ui;

    //线程
    QThread *thread;
    //server处理放到线程中
    ServerOperate *operate;
};
#endif // SERVERWIDGET_H
