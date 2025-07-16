#include "gui/BaseWidget.h"
#include <QApplication>

BaseWidget::BaseWidget(QWidget* parent)
    : QWidget(parent)
{
    setupBaseStyles();
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
    groupBox->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #cccccc;"
        "    border-radius: 8px;"
        "    margin-top: 1ex;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
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
    button->setStyleSheet(
        "QPushButton {"
        "    background-color: #0078d4;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #106ebe;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005a9e;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
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
    // 设置基础样式
    setStyleSheet(
        "QWidget {"
        "    font-family: 'Microsoft YaHei', 'SimHei', sans-serif;"
        "    font-size: 9pt;"
        "}"
        "QLineEdit {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 4px;"
        "    padding: 6px;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #0078d4;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "}"
        "QComboBox {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 4px;"
        "    padding: 4px 8px;"
        "    background-color: white;"
        "}"
        "QSpinBox {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 4px;"
        "    padding: 4px;"
        "    background-color: white;"
        "}"
    );
    
    applyDefaultStyles();
}
