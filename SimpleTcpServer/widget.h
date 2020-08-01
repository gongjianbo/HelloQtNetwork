#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    //初始化server操作
    void initServer();
    //close server
    void closeServer();
    //更新当前状态
    void updateState();

private:
    Ui::Widget *ui;
    //server用于监听端口，获取新的tcp连接的描述符
    QTcpServer *server;
    //存储已连接的socket对象
    QList<QTcpSocket*> clientList;
};
#endif // WIDGET_H
