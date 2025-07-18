#pragma once

#include <QSettings>
#include <QString>
#include <QUrl>

/**
 * @class AppSettings
 * @brief 应用程序设置管理器，提供便捷的设置访问方法
 */
class AppSettings
{
public:
    /**
     * @brief 获取单例实例
     * @return AppSettings实例的引用
     */
    static AppSettings& instance();

    /**
     * @brief 检查是否启用自动打开URL功能
     * @return 如果启用返回true，否则返回false
     */
    bool isAutoOpenUrlEnabled() const;

    /**
     * @brief 设置自动打开URL功能状态
     * @param enabled 是否启用
     */
    void setAutoOpenUrlEnabled(bool enabled);

    /**
     * @brief 检查文本是否为有效的URL
     * @param text 要检查的文本
     * @return 如果是有效URL返回true，否则返回false
     */
    static bool isValidUrl(const QString& text);

    /**
     * @brief 如果启用了自动打开功能且文本是URL，则打开URL
     * @param text 识别结果文本
     * @return 如果打开了URL返回true，否则返回false
     */
    bool tryAutoOpenUrl(const QString& text) const;

    /**
     * @brief 获取其他识别设置
     */
    bool isAutoRecognizeEnabled() const;
    int getRecognitionTimeout() const;
    bool isAutoCopyEnabled() const;

private:
    AppSettings() = default;
    ~AppSettings() = default;
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;

    QSettings m_settings;
};
