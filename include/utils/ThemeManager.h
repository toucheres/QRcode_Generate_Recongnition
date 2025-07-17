#pragma once

#include <QObject>
#include <QString>
#include <QApplication>
#include <QPalette>

/**
 * @class ThemeManager
 * @brief 主题管理器，负责应用程序的主题切换和管理
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class Theme
    {
        System,     // 跟随系统主题
        Light,      // 浅色主题
        Dark        // 深色主题
    };

    static ThemeManager* instance();
    
    /**
     * @brief 设置当前主题
     * @param theme 主题类型
     */
    void setTheme(Theme theme);
    
    /**
     * @brief 获取当前主题设置
     * @return 当前主题类型
     */
    Theme currentTheme() const { return m_currentTheme; }
    
    /**
     * @brief 检测当前是否为深色主题
     * @return true 如果是深色主题
     */
    bool isDarkTheme() const;
    
    /**
     * @brief 获取主题名称
     * @param theme 主题类型
     * @return 主题名称字符串
     */
    static QString getThemeName(Theme theme);
    
    /**
     * @brief 从字符串获取主题类型
     * @param name 主题名称
     * @return 主题类型
     */
    static Theme getThemeFromName(const QString& name);
    
    /**
     * @brief 加载保存的主题设置
     */
    void loadSettings();
    
    /**
     * @brief 保存主题设置
     */
    void saveSettings();

signals:
    /**
     * @brief 主题改变信号
     * @param isDark 是否为深色主题
     */
    void themeChanged(bool isDark);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    
    /**
     * @brief 应用主题到应用程序
     */
    void applyTheme();
    
    /**
     * @brief 检测系统主题
     * @return true 如果系统为深色主题
     */
    bool detectSystemDarkTheme() const;

private:
    static ThemeManager* s_instance;
    Theme m_currentTheme;
    bool m_currentIsDark;
};
