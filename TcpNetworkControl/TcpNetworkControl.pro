QT += core
QT += gui
QT += widgets
QT += network
QT += concurrent

CONFIG += c++17
CONFIG += utf8_source

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    ControlClient.cpp \
    ControlServer.cpp \
    MyTcpServer.cpp \
    MyTcpSocket.cpp \
    ProtocolParser.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    ControlClient.h \
    ControlServer.h \
    MainWindow.h \
    MyTcpServer.h \
    MyTcpSocket.h \
    ProtocolParser.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Readme.txt
