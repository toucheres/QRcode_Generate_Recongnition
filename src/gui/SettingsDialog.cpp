#include "gui/SettingsDialog.h"
#include "utils/ThemeManager.h"
#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_originalTheme(ThemeManager::instance()->currentTheme())
{
    setWindowTitle("设置");
    setModal(true);
    setFixedSize(450, 500);
    
    setupUI();
    loadSettings();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 主题设置组
    m_themeGroup = new QGroupBox("主题设置");
    QFormLayout* themeLayout = new QFormLayout(m_themeGroup);
    
    m_themeCombo = new QComboBox();
    m_themeCombo->addItem(ThemeManager::getThemeName(ThemeManager::Theme::System));
    m_themeCombo->addItem(ThemeManager::getThemeName(ThemeManager::Theme::Light));
    m_themeCombo->addItem(ThemeManager::getThemeName(ThemeManager::Theme::Dark));
    
    m_themePreviewLabel = new QLabel("预览：当前主题将立即应用");
    m_themePreviewLabel->setStyleSheet("QLabel { color: #666666; font-style: italic; }");
    
    themeLayout->addRow("主题选择:", m_themeCombo);
    themeLayout->addRow("", m_themePreviewLabel);
    
    mainLayout->addWidget(m_themeGroup);
    
    // 识别设置组
    m_recognitionGroup = new QGroupBox("识别设置");
    QFormLayout* recognitionLayout = new QFormLayout(m_recognitionGroup);
    
    m_autoRecognizeCheckBox = new QCheckBox("启用摄像头自动识别");
    m_autoRecognizeCheckBox->setChecked(true);
    
    m_recognitionTimeoutSpinBox = new QSpinBox();
    m_recognitionTimeoutSpinBox->setRange(1000, 10000);
    m_recognitionTimeoutSpinBox->setValue(5000);
    m_recognitionTimeoutSpinBox->setSuffix(" ms");
    
    m_autoCopyCheckBox = new QCheckBox("自动复制识别结果到剪贴板");
    m_autoCopyCheckBox->setChecked(true);
    
    m_autoOpenUrlCheckBox = new QCheckBox("识别结果为网址时自动快捷打开");
    m_autoOpenUrlCheckBox->setChecked(true);
    m_autoOpenUrlCheckBox->setToolTip("当识别到的内容是网址(http/https)时，提供快捷打开选项");
    
    recognitionLayout->addRow(m_autoRecognizeCheckBox);
    recognitionLayout->addRow("识别超时:", m_recognitionTimeoutSpinBox);
    recognitionLayout->addRow(m_autoCopyCheckBox);
    recognitionLayout->addRow(m_autoOpenUrlCheckBox);
    
    mainLayout->addWidget(m_recognitionGroup);
    
    // 界面设置组
    m_interfaceGroup = new QGroupBox("界面设置");
    QFormLayout* interfaceLayout = new QFormLayout(m_interfaceGroup);
    
    m_showStatusBarCheckBox = new QCheckBox("显示状态栏");
    m_showStatusBarCheckBox->setChecked(true);
    
    m_rememberWindowSizeCheckBox = new QCheckBox("记住窗口大小和位置");
    m_rememberWindowSizeCheckBox->setChecked(true);
    
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(8, 20);
    m_fontSizeSpinBox->setValue(9);
    m_fontSizeSpinBox->setSuffix(" pt");
    
    interfaceLayout->addRow(m_showStatusBarCheckBox);
    interfaceLayout->addRow(m_rememberWindowSizeCheckBox);
    interfaceLayout->addRow("字体大小:", m_fontSizeSpinBox);
    
    mainLayout->addWidget(m_interfaceGroup);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_resetButton = new QPushButton("重置默认");
    m_applyButton = new QPushButton("应用");
    m_cancelButton = new QPushButton("取消");
    m_okButton = new QPushButton("确定");
    
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onThemeChanged);
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onReject);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onReset);
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    
    // 加载主题设置
    ThemeManager::Theme currentTheme = ThemeManager::instance()->currentTheme();
    m_themeCombo->setCurrentText(ThemeManager::getThemeName(currentTheme));
    
    // 加载识别设置
    m_autoRecognizeCheckBox->setChecked(settings.value("recognition/autoRecognize", true).toBool());
    m_recognitionTimeoutSpinBox->setValue(settings.value("recognition/timeout", 5000).toInt());
    m_autoCopyCheckBox->setChecked(settings.value("recognition/autoCopy", true).toBool());
    m_autoOpenUrlCheckBox->setChecked(settings.value("recognition/autoOpenUrl", true).toBool());
    
    // 加载界面设置
    m_showStatusBarCheckBox->setChecked(settings.value("interface/showStatusBar", true).toBool());
    m_rememberWindowSizeCheckBox->setChecked(settings.value("interface/rememberWindowSize", true).toBool());
    m_fontSizeSpinBox->setValue(settings.value("interface/fontSize", 9).toInt());
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    
    // 保存识别设置
    settings.setValue("recognition/autoRecognize", m_autoRecognizeCheckBox->isChecked());
    settings.setValue("recognition/timeout", m_recognitionTimeoutSpinBox->value());
    settings.setValue("recognition/autoCopy", m_autoCopyCheckBox->isChecked());
    settings.setValue("recognition/autoOpenUrl", m_autoOpenUrlCheckBox->isChecked());
    
    // 保存界面设置
    settings.setValue("interface/showStatusBar", m_showStatusBarCheckBox->isChecked());
    settings.setValue("interface/rememberWindowSize", m_rememberWindowSizeCheckBox->isChecked());
    settings.setValue("interface/fontSize", m_fontSizeSpinBox->value());
}

void SettingsDialog::applySettings()
{
    // 应用主题设置
    QString themeText = m_themeCombo->currentText();
    ThemeManager::Theme theme = ThemeManager::getThemeFromName(themeText);
    ThemeManager::instance()->setTheme(theme);
    
    // 保存其他设置
    saveSettings();
    
    // 更新应用程序字体大小
    QFont font = QApplication::font();
    font.setPointSize(m_fontSizeSpinBox->value());
    QApplication::setFont(font);
}

void SettingsDialog::resetToDefaults()
{
    // 重置为默认值
    m_themeCombo->setCurrentText(ThemeManager::getThemeName(ThemeManager::Theme::System));
    m_autoRecognizeCheckBox->setChecked(true);
    m_recognitionTimeoutSpinBox->setValue(5000);
    m_autoCopyCheckBox->setChecked(true);
    m_autoOpenUrlCheckBox->setChecked(true);
    m_showStatusBarCheckBox->setChecked(true);
    m_rememberWindowSizeCheckBox->setChecked(true);
    m_fontSizeSpinBox->setValue(9);
}

void SettingsDialog::onThemeChanged()
{
    // 立即预览主题更改
    QString themeText = m_themeCombo->currentText();
    ThemeManager::Theme theme = ThemeManager::getThemeFromName(themeText);
    ThemeManager::instance()->setTheme(theme);
    
    // 更新预览标签
    QString previewText = QString("预览：已切换到%1").arg(themeText);
    m_themePreviewLabel->setText(previewText);
}

void SettingsDialog::onAccept()
{
    applySettings();
    accept();
}

void SettingsDialog::onReject()
{
    // 恢复原始主题
    ThemeManager::instance()->setTheme(m_originalTheme);
    reject();
}

void SettingsDialog::onApply()
{
    applySettings();
    m_originalTheme = ThemeManager::instance()->currentTheme();
    
    QMessageBox::information(this, "设置", "设置已应用");
}

void SettingsDialog::onReset()
{
    int ret = QMessageBox::question(this, "重置设置", 
                                   "确定要重置所有设置为默认值吗？",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        resetToDefaults();
        QMessageBox::information(this, "设置", "设置已重置为默认值");
    }
}
