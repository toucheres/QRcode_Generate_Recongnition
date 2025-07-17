#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

/**
 * @class BaseWidget
 * @brief GUI组件基类，提供通用的界面元素和布局
 */
class BaseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BaseWidget(QWidget* parent = nullptr);
    virtual ~BaseWidget() = default;

protected slots:
    /**
     * @brief 主题变化时调用
     * @param isDark 是否为深色主题
     */
    virtual void onThemeChanged(bool isDark);

protected:
    /**
     * @brief 创建垂直布局
     * @param parent 父控件
     * @param margins 边距
     * @param spacing 间距
     * @return 创建的布局
     */
    QVBoxLayout* createVBoxLayout(QWidget* parent = nullptr, int margins = 10, int spacing = 10);

    /**
     * @brief 创建水平布局
     * @param parent 父控件
     * @param margins 边距
     * @param spacing 间距
     * @return 创建的布局
     */
    QHBoxLayout* createHBoxLayout(QWidget* parent = nullptr, int margins = 10, int spacing = 10);

    /**
     * @brief 创建分组框
     * @param title 标题
     * @param parent 父控件
     * @return 创建的分组框
     */
    QGroupBox* createGroupBox(const QString& title, QWidget* parent = nullptr);

    /**
     * @brief 创建标签
     * @param text 文本
     * @param parent 父控件
     * @return 创建的标签
     */
    QLabel* createLabel(const QString& text, QWidget* parent = nullptr);

    /**
     * @brief 创建按钮
     * @param text 按钮文本
     * @param parent 父控件
     * @return 创建的按钮
     */
    QPushButton* createButton(const QString& text, QWidget* parent = nullptr);

    /**
     * @brief 设置控件样式
     * @param widget 控件
     * @param styleSheet 样式表
     */
    void setWidgetStyle(QWidget* widget, const QString& styleSheet);

    /**
     * @brief 应用默认样式
     */
    virtual void applyDefaultStyles();

    /**
     * @brief 检测当前是否为深色主题
     * @return 如果是深色主题返回true，否则返回false
     */
    bool isDarkTheme() const;

private:
    void setupBaseStyles();
};
