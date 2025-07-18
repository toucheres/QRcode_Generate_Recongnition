#include "utils/AppSettings.h"
#include <QUrl>
#include <QDesktopServices>
#include <QDebug>
#include <QMessageBox>
#include <QApplication>

AppSettings& AppSettings::instance()
{
    static AppSettings instance;
    return instance;
}

bool AppSettings::isAutoOpenUrlEnabled() const
{
    return m_settings.value("recognition/autoOpenUrl", true).toBool();
}

void AppSettings::setAutoOpenUrlEnabled(bool enabled)
{
    m_settings.setValue("recognition/autoOpenUrl", enabled);
}

bool AppSettings::isValidUrl(const QString& text)
{
    if (text.isEmpty()) {
        return false;
    }

    // 创建QUrl对象
    QUrl url(text.trimmed());
    
    // 检查URL是否有效且为HTTP/HTTPS协议
    return url.isValid() && 
           !url.host().isEmpty() && 
           (url.scheme() == "http" || url.scheme() == "https");
}

bool AppSettings::tryAutoOpenUrl(const QString& text) const
{
    // 检查是否启用了自动打开功能
    if (!isAutoOpenUrlEnabled()) {
        return false;
    }

    // 检查文本是否为有效URL
    if (!isValidUrl(text)) {
        return false;
    }

    // 尝试打开URL
    QUrl url(text.trimmed());
    bool success = QDesktopServices::openUrl(url);
    
    if (success) {
        qDebug() << "自动打开URL:" << url.toString();
    } else {
        qDebug() << "无法打开URL:" << url.toString();
        
        // 显示错误消息（可选）
        QMessageBox::warning(
            qApp->activeWindow(),
            "打开网址失败",
            QString("无法打开网址：%1\n\n请检查网址是否正确或手动复制到浏览器中打开。").arg(url.toString())
        );
    }
    
    return success;
}

bool AppSettings::isAutoRecognizeEnabled() const
{
    return m_settings.value("recognition/autoRecognize", true).toBool();
}

int AppSettings::getRecognitionTimeout() const
{
    return m_settings.value("recognition/timeout", 5000).toInt();
}

bool AppSettings::isAutoCopyEnabled() const
{
    return m_settings.value("recognition/autoCopy", true).toBool();
}
