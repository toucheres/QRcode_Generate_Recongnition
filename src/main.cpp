#include <QApplication>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置默认编码为UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif
    
    // 设置应用程序信息
    app.setApplicationName("QRcode生成识别器");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("MyCompany");
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}
