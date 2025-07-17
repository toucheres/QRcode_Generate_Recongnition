#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextCodec>
#include "gui/MainWindow.h"
// TODO: PDF417 + Chinese character crash issue
// Steps to reproduce:
//   1. Generate or recognize a PDF417 barcode containing Chinese characters.
//   2. Observe application behavior.
//
// Expected behavior:
//   - The application should correctly process PDF417 barcodes with Chinese characters.
//
// Actual behavior:
//   - The application crashes when handling PDF417 barcodes with Chinese characters.
//
// Potential workarounds:
//   - Ensure all relevant libraries support UTF-8 encoding.
//   - Check for updates or patches in the barcode library.
//   - Consider preprocessing text to avoid problematic characters.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置UTF-8编码支持，确保中文正确处理
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
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
