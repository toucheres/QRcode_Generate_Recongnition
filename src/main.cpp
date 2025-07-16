#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include "gui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("QR码生成识别器");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("SCU-CS");
    app.setOrganizationDomain("scu-cs.org");
    
    // 设置应用程序图标
    app.setWindowIcon(QIcon(":/toucher.jpg"));
    
    // 确保配置目录存在
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}
