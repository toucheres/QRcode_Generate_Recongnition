#include <QApplication>
#include <QTextCodec>
#include "mainwindow.h"
// TODO 绘制边框
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);   
    app.setApplicationName("多格式二维码生成识别器");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("SCU-CS");
    app.setWindowIcon(QIcon{":/toucher.jpg"});
    MainWindow window;
    window.show();
    return app.exec();
}
