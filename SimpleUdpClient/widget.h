#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

//udp demo
class Widget : public QWidget
{
    Q_OBJECT
public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    //初始化client操作
    void initClient();
    //更新当前状态
    void updateState();

private:
    Ui::Widget *ui;
    //socket对象
    QUdpSocket *udpSocket;
};
#endif // WIDGET_H
