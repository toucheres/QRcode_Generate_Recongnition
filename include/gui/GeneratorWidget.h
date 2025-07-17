#pragma once

#include "gui/BaseWidget.h"
#include "core/QRCodeGenerator.h"
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>

/**
 * @class GeneratorWidget
 * @brief 二维码生成界面组件
 */
class GeneratorWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit GeneratorWidget(QWidget* parent = nullptr);
    ~GeneratorWidget() = default;

    /**
     * @brief 获取当前生成配置
     * @return 生成配置
     */
    QRCodeGenerator::GenerationConfig getConfig() const;

    /**
     * @brief 设置生成配置
     * @param config 生成配置
     */
    void setConfig(const QRCodeGenerator::GenerationConfig& config);

signals:
    /**
     * @brief 请求生成二维码信号
     */
    void generateRequested();

    /**
     * @brief 请求保存二维码信号
     * @param pixmap 要保存的图像
     */
    void saveRequested(const QPixmap& pixmap);

public slots:
    /**
     * @brief 显示生成的二维码
     * @param pixmap 生成的二维码图像
     */
    void showGeneratedQRCode(const QPixmap& pixmap);

    /**
     * @brief 显示错误信息
     * @param error 错误信息
     */
    void showError(const QString& error);

private slots:
    void onGenerateClicked();
    void onSaveClicked();
    void onSelectLogoClicked();
    void onEmbedLogoToggled(bool enabled);
    void onLogoSizeChanged(int size);
    void onFormatChanged(); // 格式变化槽函数
    void onCustomTextToggled(bool enabled); // 自定义文本切换
    void onCustomTextChanged(); // 自定义文本内容变化
    void onAutoGenerate(); // 自动生成槽函数

protected:
    void applyDefaultStyles() override;

private:
    void setupUI();
    void updateLogoPreview();
    void updateLogoControls();
    void connectSignals();     // 连接信号槽
    void updateFormatInfo();   // 更新格式信息
    void adjustFormatInfoHeight(); // 调整格式说明区域高度

private:
    // 输入控件
    QLineEdit* m_textInput;
    QComboBox* m_formatCombo;
    QComboBox* m_errorCorrectionCombo;
    QSpinBox* m_sizeSpinBox;
    
    // Logo相关控件
    QCheckBox* m_embedLogoCheckBox;
    QPushButton* m_selectLogoButton;
    QLabel* m_logoPreviewLabel;
    QSlider* m_logoSizeSlider;
    QLabel* m_logoSizeLabel;
    
    // 自定义文本相关控件
    QCheckBox* m_customTextCheckBox;
    QLineEdit* m_customTextInput;
    QComboBox* m_textPositionCombo;
    QSpinBox* m_textSizeSpinBox;
    QComboBox* m_textColorCombo;
    
    // 自动生成控件
    QCheckBox* m_autoGenerateCheckBox;
    
    // 操作按钮
    QPushButton* m_generateButton;
    QPushButton* m_saveButton;
    
    // 显示控件
    QLabel* m_qrCodeLabel;
    QLabel* m_statusLabel;
    QScrollArea* m_formatInfoScrollArea;  // 格式说明滚动区域
    QLabel* m_formatInfoLabel;            // 格式说明标签
    
    // 数据
    QPixmap m_logoPixmap;
    QPixmap m_currentQRCode;
    QRCodeGenerator* m_generator;
};
