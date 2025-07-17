#include "gui/BaseWidget.h"
#include "utils/ThemeManager.h"
#include <QApplication>
#include <QPalette>

BaseWidget::BaseWidget(QWidget* parent)
    : QWidget(parent)
{
    setupBaseStyles();
    
    // 连接主题管理器信号
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &BaseWidget::onThemeChanged);
}

QVBoxLayout* BaseWidget::createVBoxLayout(QWidget* parent, int margins, int spacing)
{
    QVBoxLayout* layout = new QVBoxLayout(parent);
    layout->setContentsMargins(margins, margins, margins, margins);
    layout->setSpacing(spacing);
    return layout;
}

QHBoxLayout* BaseWidget::createHBoxLayout(QWidget* parent, int margins, int spacing)
{
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(margins, margins, margins, margins);
    layout->setSpacing(spacing);
    return layout;
}

QGroupBox* BaseWidget::createGroupBox(const QString& title, QWidget* parent)
{
    QGroupBox* groupBox = new QGroupBox(title, parent);
    
    // 使用主题管理器检测主题
    bool isDarkTheme = ThemeManager::instance()->isDarkTheme();
    
    QString borderColor = isDarkTheme ? "#555555" : "#cccccc";
    QString textColor = isDarkTheme ? "#ffffff" : "#000000";
    
    groupBox->setStyleSheet(
        QString("QGroupBox {"
        "    font-weight: bold;"
        "    color: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 8px;"
        "    margin-top: 1ex;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "    color: %1;"
        "}").arg(textColor, borderColor)
    );
    return groupBox;
}

QLabel* BaseWidget::createLabel(const QString& text, QWidget* parent)
{
    QLabel* label = new QLabel(text, parent);
    label->setWordWrap(true);
    return label;
}

QPushButton* BaseWidget::createButton(const QString& text, QWidget* parent)
{
    QPushButton* button = new QPushButton(text, parent);
    
    // 检测系统主题
    QPalette palette = QApplication::palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    // 根据主题设置按钮颜色
    QString buttonBgColor = isDarkTheme ? "#0078d4" : "#0078d4";
    QString buttonHoverColor = isDarkTheme ? "#106ebe" : "#106ebe";
    QString buttonPressedColor = isDarkTheme ? "#005a9e" : "#005a9e";
    QString buttonDisabledBgColor = isDarkTheme ? "#555555" : "#cccccc";
    QString buttonDisabledTextColor = isDarkTheme ? "#999999" : "#666666";
    
    button->setStyleSheet(
        QString("QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "}"
        "QPushButton:disabled {"
        "    background-color: %4;"
        "    color: %5;"
        "}")
        .arg(buttonBgColor, buttonHoverColor, buttonPressedColor, buttonDisabledBgColor, buttonDisabledTextColor)
    );
    return button;
}

void BaseWidget::setWidgetStyle(QWidget* widget, const QString& styleSheet)
{
    if (widget) {
        widget->setStyleSheet(styleSheet);
    }
}

void BaseWidget::applyDefaultStyles()
{
    // 子类可以重写此方法应用特定样式
}

void BaseWidget::setupBaseStyles()
{
    // 使用主题管理器检测主题
    bool isDarkTheme = ThemeManager::instance()->isDarkTheme();
    
    // 根据主题设置颜色
    QString backgroundColor = isDarkTheme ? "#2b2b2b" : "#ffffff";
    QString textColor = isDarkTheme ? "#ffffff" : "#000000";
    QString borderColor = isDarkTheme ? "#555555" : "#cccccc";
    QString borderFocusColor = isDarkTheme ? "#0078d4" : "#0078d4";
    QString placeholderColor = isDarkTheme ? "#888888" : "#999999";
    QString disabledTextColor = isDarkTheme ? "#666666" : "#cccccc";
    QString tooltipBgColor = isDarkTheme ? "#1e1e1e" : "#ffffcc";
    QString tooltipTextColor = isDarkTheme ? "#ffffff" : "#000000";
    
    // 设置基础样式
    setStyleSheet(
        QString("QWidget {"
        "    font-family: 'Microsoft YaHei', 'SimHei', sans-serif;"
        "    font-size: 9pt;"
        "    color: %1;"
        "    background-color: %2;"
        "}"
        "QLineEdit {"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    padding: 6px;"
        "    background-color: %2;"
        "    color: %1;"
        "}"
        "QLineEdit:focus {"
        "    border-color: %4;"
        "}"
        "QLineEdit:disabled {"
        "    color: %6;"
        "    background-color: %2;"
        "}"
        "QLineEdit::placeholder {"
        "    color: %5;"
        "}"
        "QTextEdit {"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    background-color: %2;"
        "    color: %1;"
        "    selection-background-color: %4;"
        "}"
        "QTextEdit:disabled {"
        "    color: %6;"
        "}"
        "QTextEdit::placeholder {"
        "    color: %5;"
        "}"
        "QPlainTextEdit {"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    background-color: %2;"
        "    color: %1;"
        "    selection-background-color: %4;"
        "}"
        "QPlainTextEdit:disabled {"
        "    color: %6;"
        "}"
        "QPlainTextEdit::placeholder {"
        "    color: %5;"
        "}"
        "QComboBox {"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    padding: 4px 8px;"
        "    background-color: %2;"
        "    color: %1;"
        "}"
        "QComboBox:disabled {"
        "    color: %6;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: %2;"
        "    color: %1;"
        "    border: 1px solid %3;"
        "    selection-background-color: %4;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    background-color: %2;"
        "}"
        "QComboBox::down-arrow {"
        "    border: 1px solid %1;"
        "    border-radius: 2px;"
        "}"
        "QSpinBox {"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "    background-color: %2;"
        "    color: %1;"
        "}"
        "QDoubleSpinBox {"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "    background-color: %2;"
        "    color: %1;"
        "}"
        "QLabel {"
        "    color: %1;"
        "    background-color: transparent;"
        "}"
        "QCheckBox {"
        "    color: %1;"
        "    background-color: transparent;"
        "}"
        "QRadioButton {"
        "    color: %1;"
        "    background-color: transparent;"
        "}"
        "QSlider::groove:horizontal {"
        "    border: 1px solid %3;"
        "    height: 8px;"
        "    background-color: %2;"
        "    border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background-color: %4;"
        "    border: 1px solid %3;"
        "    width: 18px;"
        "    border-radius: 9px;"
        "}"
        "QScrollArea {"
        "    background-color: %2;"
        "    color: %1;"
        "}"
        "QScrollBar:vertical {"
        "    background-color: %2;"
        "    width: 12px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: %3;"
        "    border-radius: 6px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: %4;"
        "}"
        "QToolTip {"
        "    background-color: %7;"
        "    color: %8;"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    padding: 4px 8px;"
        "}"
        "QSpinBox:disabled, QDoubleSpinBox:disabled {"
        "    color: %6;"
        "}"
        "QCheckBox:disabled, QRadioButton:disabled {"
        "    color: %6;"
        "}")
        .arg(textColor, backgroundColor, borderColor, borderFocusColor, placeholderColor, disabledTextColor, tooltipBgColor, tooltipTextColor)
    );
    
    applyDefaultStyles();
}

void BaseWidget::onThemeChanged(bool isDark)
{
    Q_UNUSED(isDark)
    // 重新应用样式
    setupBaseStyles();
}

bool BaseWidget::isDarkTheme() const
{
    return ThemeManager::instance()->isDarkTheme();
}
