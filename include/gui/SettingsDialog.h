#pragma once

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "utils/ThemeManager.h"

/**
 * @class SettingsDialog
 * @brief 设置对话框，允许用户配置应用程序设置
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onThemeChanged();
    void onAccept();
    void onReject();
    void onApply();
    void onReset();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void applySettings();
    void resetToDefaults();
    
    // 主题设置
    QGroupBox* m_themeGroup;
    QComboBox* m_themeCombo;
    QLabel* m_themePreviewLabel;
    
    // 识别设置
    QGroupBox* m_recognitionGroup;
    QCheckBox* m_autoRecognizeCheckBox;
    QSpinBox* m_recognitionTimeoutSpinBox;
    QCheckBox* m_autoCopyCheckBox;
    QCheckBox* m_autoOpenUrlCheckBox;  // 新增：自动打开网址选项
    
    // 界面设置
    QGroupBox* m_interfaceGroup;
    QCheckBox* m_showStatusBarCheckBox;
    QCheckBox* m_rememberWindowSizeCheckBox;
    QSpinBox* m_fontSizeSpinBox;
    
    // 按钮
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;
    
    // 原始设置值（用于取消时恢复）
    ThemeManager::Theme m_originalTheme;
};
