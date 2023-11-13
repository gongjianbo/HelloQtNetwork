# HelloQtNetwork

Show the use of Qt network module. 展示Qt网络模块的使用

# Environment （开发环境）

（2023-11-13）Qt5.15.2 + MSVC2019/2022 32bit/64bit

# Note （备注）

（2023-03-03）Qt5.8 之后默认使用系统代理，有时可能影响 Qt 网络访问，可以设置全局或者 Socket 的代理模式，如全局禁用代理 QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy)。
