#include <QApplication>

#include "ServerWidget.h"
#include "ClientWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //客户端作为发送端
    ClientWidget c_w;
    c_w.move(100,100);
    c_w.show();
    //服务端作为接收端
    ServerWidget s_w;
    s_w.move(600,100);
    s_w.show();

    return a.exec();
}
