#include <QApplication>
#include <QTextCodec>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);   
    app.setApplicationName("QRcode生成识别器");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("SCU-CS");
    app.setWindowIcon(QIcon{":/toucher.jpg"});
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    return app.exec();
}
