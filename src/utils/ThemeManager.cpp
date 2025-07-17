#include "utils/ThemeManager.h"
#include <QSettings>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QDebug>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    static ThemeManager instance;
    return &instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(Theme::System)
    , m_currentIsDark(false)
{
    m_currentIsDark = detectSystemDarkTheme();
}

void ThemeManager::setTheme(Theme theme)
{
    if (m_currentTheme == theme) {
        return;
    }
    
    m_currentTheme = theme;
    applyTheme();
    saveSettings();
    
    emit themeChanged(m_currentIsDark);
}

bool ThemeManager::isDarkTheme() const
{
    return m_currentIsDark;
}

QString ThemeManager::getThemeName(Theme theme)
{
    switch (theme) {
        case Theme::System: return "跟随系统";
        case Theme::Light: return "浅色主题";
        case Theme::Dark: return "深色主题";
    }
    return "未知主题";
}

ThemeManager::Theme ThemeManager::getThemeFromName(const QString& name)
{
    if (name == "跟随系统" || name == "System") return Theme::System;
    if (name == "浅色主题" || name == "Light") return Theme::Light;
    if (name == "深色主题" || name == "Dark") return Theme::Dark;
    return Theme::System;
}

void ThemeManager::loadSettings()
{
    QSettings settings;
    QString themeName = settings.value("theme", "跟随系统").toString();
    Theme theme = getThemeFromName(themeName);
    
    m_currentTheme = theme;
    applyTheme();
}

void ThemeManager::saveSettings()
{
    QSettings settings;
    settings.setValue("theme", getThemeName(m_currentTheme));
}

void ThemeManager::applyTheme()
{
    bool newIsDark = false;
    
    switch (m_currentTheme) {
        case Theme::System:
            newIsDark = detectSystemDarkTheme();
            break;
        case Theme::Light:
            newIsDark = false;
            break;
        case Theme::Dark:
            newIsDark = true;
            break;
    }
    
    // 只有在主题真正改变时才发出信号
    bool themeChanged = (newIsDark != m_currentIsDark);
    m_currentIsDark = newIsDark;
    
    // 设置应用程序样式
    if (m_currentIsDark) {
        // 深色主题
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(45, 45, 45));
        darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
        darkPalette.setColor(QPalette::Base, QColor(30, 30, 30));
        darkPalette.setColor(QPalette::AlternateBase, QColor(60, 60, 60));
        darkPalette.setColor(QPalette::ToolTipBase, QColor(0, 0, 0));
        darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
        darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
        darkPalette.setColor(QPalette::Button, QColor(60, 60, 60));
        darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
        darkPalette.setColor(QPalette::BrightText, QColor(255, 0, 0));
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
        
        QApplication::setPalette(darkPalette);
        qDebug() << "Applied dark theme";
    } else {
        // 浅色主题 - 恢复默认调色板
        QApplication::setPalette(QApplication::style()->standardPalette());
        qDebug() << "Applied light theme";
    }
    
    if (themeChanged) {
        emit this->themeChanged(m_currentIsDark);
    }
}

bool ThemeManager::detectSystemDarkTheme() const
{
    // 检测系统主题
    QPalette defaultPalette = QApplication::style()->standardPalette();
    QColor windowColor = defaultPalette.color(QPalette::Window);
    
    // 如果窗口背景色的亮度小于128，认为是深色主题
    return windowColor.lightness() < 128;
}
