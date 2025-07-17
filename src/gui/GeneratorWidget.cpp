#include "gui/GeneratorWidget.h"
#include "utils/AppUtils.h"
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QPalette>
#include <QScrollArea>
#include <QFrame>
#include <QFontMetrics>
#include <QTimer>

GeneratorWidget::GeneratorWidget(QWidget* parent)
    : BaseWidget(parent)
    , m_generator(new QRCodeGenerator())
{
    setupUI();
}

QRCodeGenerator::GenerationConfig GeneratorWidget::getConfig() const
{
    QRCodeGenerator::GenerationConfig config;
    config.text = m_textInput->text();
    config.size = QSize(m_sizeSpinBox->value(), m_sizeSpinBox->value());
    
    // 设置错误纠正级别
    QString ecText = m_errorCorrectionCombo->currentText();
    if (ecText == "低 (L)") config.errorCorrection = QRCodeGenerator::ErrorCorrectionLevel::Low;
    else if (ecText == "中 (M)") config.errorCorrection = QRCodeGenerator::ErrorCorrectionLevel::Medium;
    else if (ecText == "高 (Q)") config.errorCorrection = QRCodeGenerator::ErrorCorrectionLevel::Quartile;
    else if (ecText == "最高 (H)") config.errorCorrection = QRCodeGenerator::ErrorCorrectionLevel::High;
    
    // 设置条码格式
    QString formatText = m_formatCombo->currentText();
    if (formatText.startsWith("QR Code")) config.format = ZXing::BarcodeFormat::QRCode;
    else if (formatText.startsWith("Data Matrix")) config.format = ZXing::BarcodeFormat::DataMatrix;
    else if (formatText.startsWith("PDF417")) config.format = ZXing::BarcodeFormat::PDF417;
    else if (formatText.startsWith("Aztec")) config.format = ZXing::BarcodeFormat::Aztec;
    else if (formatText.startsWith("Code 128")) config.format = ZXing::BarcodeFormat::Code128;
    else if (formatText.startsWith("Code 39")) config.format = ZXing::BarcodeFormat::Code39;
    else if (formatText.startsWith("Code 93")) config.format = ZXing::BarcodeFormat::Code93;
    else if (formatText.startsWith("Codabar")) config.format = ZXing::BarcodeFormat::Codabar;
    else if (formatText.startsWith("EAN-8")) config.format = ZXing::BarcodeFormat::EAN8;
    else if (formatText.startsWith("EAN-13")) config.format = ZXing::BarcodeFormat::EAN13;
    else if (formatText.startsWith("UPC-A")) config.format = ZXing::BarcodeFormat::UPCA;
    else if (formatText.startsWith("UPC-E")) config.format = ZXing::BarcodeFormat::UPCE;
    else if (formatText.startsWith("ITF")) config.format = ZXing::BarcodeFormat::ITF;
    else config.format = ZXing::BarcodeFormat::QRCode; // 默认QR码
    
    // 设置自定义文本配置
    config.enableCustomText = m_customTextCheckBox->isChecked();
    config.customText = m_customTextInput->text();
    
    // 设置文本位置
    QString positionText = m_textPositionCombo->currentText();
    if (positionText == "底部") config.textPosition = QRCodeGenerator::TextPosition::Bottom;
    else if (positionText == "顶部") config.textPosition = QRCodeGenerator::TextPosition::Top;
    else if (positionText == "左侧") config.textPosition = QRCodeGenerator::TextPosition::Left;
    else if (positionText == "右侧") config.textPosition = QRCodeGenerator::TextPosition::Right;
    
    // 设置字体大小
    config.textSize = m_textSizeSpinBox->value();
    
    // 设置文本颜色
    QString colorText = m_textColorCombo->currentText();
    if (colorText == "黑色") config.textColor = Qt::black;
    else if (colorText == "白色") config.textColor = Qt::white;
    else if (colorText == "红色") config.textColor = Qt::red;
    else if (colorText == "蓝色") config.textColor = Qt::blue;
    else if (colorText == "绿色") config.textColor = Qt::green;
    else if (colorText == "灰色") config.textColor = Qt::gray;
    else config.textColor = Qt::black; // 默认黑色
    
    return config;
}

void GeneratorWidget::setConfig(const QRCodeGenerator::GenerationConfig& config)
{
    m_textInput->setText(config.text);
    m_sizeSpinBox->setValue(config.size.width());
    
    // 设置错误纠正级别
    switch (config.errorCorrection) {
        case QRCodeGenerator::ErrorCorrectionLevel::Low:
            m_errorCorrectionCombo->setCurrentText("低 (L)");
            break;
        case QRCodeGenerator::ErrorCorrectionLevel::Medium:
            m_errorCorrectionCombo->setCurrentText("中 (M)");
            break;
        case QRCodeGenerator::ErrorCorrectionLevel::Quartile:
            m_errorCorrectionCombo->setCurrentText("高 (Q)");
            break;
        case QRCodeGenerator::ErrorCorrectionLevel::High:
            m_errorCorrectionCombo->setCurrentText("最高 (H)");
            break;
    }
    
    // 设置条码格式
    switch (config.format) {
        case ZXing::BarcodeFormat::QRCode:
            m_formatCombo->setCurrentText("QR Code - 二维码 (支持汉字、网址、文本)");
            break;
        case ZXing::BarcodeFormat::DataMatrix:
            m_formatCombo->setCurrentText("Data Matrix - 数据矩阵 (高密度存储)");
            break;
        case ZXing::BarcodeFormat::PDF417:
            m_formatCombo->setCurrentText("PDF417 - PDF417码 (大容量文档)");
            break;
        case ZXing::BarcodeFormat::Aztec:
            m_formatCombo->setCurrentText("Aztec - 阿兹特克码 (高效二维)");
            break;
        case ZXing::BarcodeFormat::Code128:
            m_formatCombo->setCurrentText("Code 128 - 一维条码 (字母数字)");
            break;
        case ZXing::BarcodeFormat::Code39:
            m_formatCombo->setCurrentText("Code 39 - 一维条码 (大写字母数字)");
            break;
        case ZXing::BarcodeFormat::Code93:
            m_formatCombo->setCurrentText("Code 93 - 一维条码 (ASCII字符)");
            break;
        case ZXing::BarcodeFormat::Codabar:
            m_formatCombo->setCurrentText("Codabar - 库德巴码 (数字应用)");
            break;
        case ZXing::BarcodeFormat::EAN8:
            m_formatCombo->setCurrentText("EAN-8 - 欧洲商品码 (8位数字)");
            break;
        case ZXing::BarcodeFormat::EAN13:
            m_formatCombo->setCurrentText("EAN-13 - 欧洲商品码 (13位数字)");
            break;
        case ZXing::BarcodeFormat::UPCA:
            m_formatCombo->setCurrentText("UPC-A - 美国商品码 (12位数字)");
            break;
        case ZXing::BarcodeFormat::UPCE:
            m_formatCombo->setCurrentText("UPC-E - 美国商品码 (缩短版)");
            break;
        case ZXing::BarcodeFormat::ITF:
            m_formatCombo->setCurrentText("ITF - 交错25码 (偶数位数字)");
            break;
        default:
            m_formatCombo->setCurrentText("QR Code - 二维码 (支持汉字、网址、文本)");
            break;
    }
    
    // 设置自定义文本配置
    m_customTextCheckBox->setChecked(config.enableCustomText);
    m_customTextInput->setText(config.customText);
    
    // 设置文本位置
    switch (config.textPosition) {
        case QRCodeGenerator::TextPosition::Bottom:
            m_textPositionCombo->setCurrentText("底部");
            break;
        case QRCodeGenerator::TextPosition::Top:
            m_textPositionCombo->setCurrentText("顶部");
            break;
        case QRCodeGenerator::TextPosition::Left:
            m_textPositionCombo->setCurrentText("左侧");
            break;
        case QRCodeGenerator::TextPosition::Right:
            m_textPositionCombo->setCurrentText("右侧");
            break;
    }
    
    // 设置字体大小
    m_textSizeSpinBox->setValue(config.textSize);
    
    // 设置文本颜色
    if (config.textColor == Qt::black) m_textColorCombo->setCurrentText("黑色");
    else if (config.textColor == Qt::white) m_textColorCombo->setCurrentText("白色");
    else if (config.textColor == Qt::red) m_textColorCombo->setCurrentText("红色");
    else if (config.textColor == Qt::blue) m_textColorCombo->setCurrentText("蓝色");
    else if (config.textColor == Qt::green) m_textColorCombo->setCurrentText("绿色");
    else if (config.textColor == Qt::gray) m_textColorCombo->setCurrentText("灰色");
    else m_textColorCombo->setCurrentText("黑色");
}

void GeneratorWidget::showGeneratedQRCode(const QPixmap& pixmap)
{
    m_currentQRCode = pixmap;
    m_qrCodeLabel->setPixmap(pixmap.scaled(m_qrCodeLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_saveButton->setEnabled(!pixmap.isNull());
    m_statusLabel->setText("二维码生成成功");
}

void GeneratorWidget::showError(const QString& error)
{
    m_statusLabel->setText(QString("错误: %1").arg(error));
    m_saveButton->setEnabled(false);
}

void GeneratorWidget::onGenerateClicked()
{
    QString text = m_textInput->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要生成二维码的文本！");
        return;
    }

    QRCodeGenerator::GenerationConfig config = getConfig();
    
    QPixmap finalQRCode;
    
    // 如果启用了logo嵌入
    if (m_embedLogoCheckBox->isChecked() && !m_logoPixmap.isNull()) {
        // 先生成基础二维码（不包含自定义文本）
        QRCodeGenerator::GenerationConfig baseConfig = config;
        baseConfig.enableCustomText = false; // 暂时禁用自定义文本
        
        QPixmap baseQRCode = m_generator->generateQRCode(baseConfig);
        if (!baseQRCode.isNull()) {
            // 嵌入logo
            int logoSize = m_logoSizeSlider->value();
            QPixmap logoQRCode = m_generator->embedLogo(baseQRCode, m_logoPixmap, logoSize);
            
            // 如果启用了自定义文本，在logo嵌入后添加文本
            if (config.enableCustomText && !config.customText.isEmpty()) {
                finalQRCode = m_generator->addCustomText(logoQRCode, config);
            } else {
                finalQRCode = logoQRCode;
            }
        } else {
            showError(m_generator->getLastError());
            return;
        }
    } else {
        // 直接生成二维码（包含自定义文本）
        finalQRCode = m_generator->generateQRCode(config);
        if (finalQRCode.isNull()) {
            showError(m_generator->getLastError());
            return;
        }
    }
    
    showGeneratedQRCode(finalQRCode);
}

void GeneratorWidget::onSaveClicked()
{
    if (m_currentQRCode.isNull()) {
        QMessageBox::warning(this, "警告", "请先生成二维码！");
        return;
    }

    emit saveRequested(m_currentQRCode);
}

void GeneratorWidget::onSelectLogoClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择Logo图片",
        "",
        AppUtils::getImageFileFilter()
    );

    if (!fileName.isEmpty()) {
        QPixmap logo(fileName);
        if (!logo.isNull()) {
            m_logoPixmap = logo;
            updateLogoPreview();
            m_embedLogoCheckBox->setEnabled(true);
            if (!m_embedLogoCheckBox->isChecked()) {
                m_embedLogoCheckBox->setChecked(true);
            }
        } else {
            QMessageBox::warning(this, "错误", "无法加载选择的图片！");
        }
    }
}

void GeneratorWidget::onEmbedLogoToggled(bool enabled)
{
    updateLogoControls();
}

void GeneratorWidget::onLogoSizeChanged(int size)
{
    m_logoSizeLabel->setText(QString("%1%").arg(size));
    // 如果已经有生成的二维码，实时更新
    if (!m_currentQRCode.isNull() && m_embedLogoCheckBox->isChecked() && !m_logoPixmap.isNull()) {
        // 这里可以实现实时预览，但为了性能考虑，可能需要添加延时
    }
}

void GeneratorWidget::applyDefaultStyles()
{
    BaseWidget::applyDefaultStyles();
    
    // 检测系统主题
    QPalette palette = QApplication::palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    QString borderColor = isDarkTheme ? "#555555" : "#cccccc";
    QString backgroundColor = isDarkTheme ? "#3c3c3c" : "#f9f9f9";
    QString textColor = isDarkTheme ? "#ffffff" : "#000000";
    
    // 应用特定样式
    m_qrCodeLabel->setStyleSheet(
        QString("QLabel {"
        "    border: 2px dashed %1;"
        "    border-radius: 8px;"
        "    background-color: %2;"
        "    color: %3;"
        "    min-height: 300px;"
        "    text-align: center;"
        "}").arg(borderColor, backgroundColor, textColor)
    );
}

void GeneratorWidget::setupUI()
{
    setWindowTitle("二维码生成器");
    
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(10);
    
    // 左侧：设置区域 - 使用滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMaximumWidth(380);
    scrollArea->setMinimumWidth(350);
    
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* settingsLayout = createVBoxLayout(settingsWidget, 5, 5);
    
    // 基本设置组 - 紧凑布局
    QGroupBox* basicGroup = createGroupBox("基本设置");
    QGridLayout* basicGridLayout = new QGridLayout(basicGroup);
    basicGridLayout->setSpacing(8);
    basicGridLayout->setContentsMargins(10, 15, 10, 10);
    
    // 文本输入 - 占用整行
    basicGridLayout->addWidget(createLabel("输入文本:"), 0, 0, 1, 2);
    m_textInput = new QLineEdit();
    m_textInput->setPlaceholderText("请输入文本（支持中文、英文、数字、网址等）...");
    basicGridLayout->addWidget(m_textInput, 1, 0, 1, 2);
    
    // 格式选择和错误纠正级别 - 并排放置
    basicGridLayout->addWidget(createLabel("条码格式:"), 2, 0);
    basicGridLayout->addWidget(createLabel("纠错级别:"), 2, 1);
    
    m_formatCombo = new QComboBox();
    m_formatCombo->addItems({
        "QR Code - 二维码",
        "Data Matrix - 数据矩阵", 
        "PDF417 - PDF417码",
        "Aztec - 阿兹特克码",
        "Code 128 - 一维条码",
        "Code 39 - 一维条码",
        "Code 93 - 一维条码",
        "Codabar - 库德巴码",
        "EAN-8 - 欧洲商品码",
        "EAN-13 - 欧洲商品码", 
        "UPC-A - 美国商品码",
        "UPC-E - 美国商品码",
        "ITF - 交错25码"
    });
    m_formatCombo->setToolTip("选择要生成的条码格式");
    basicGridLayout->addWidget(m_formatCombo, 3, 0);
    
    m_errorCorrectionCombo = new QComboBox();
    m_errorCorrectionCombo->addItems({"低 (L)", "中 (M)", "高 (Q)", "最高 (H)"});
    m_errorCorrectionCombo->setCurrentIndex(1);
    basicGridLayout->addWidget(m_errorCorrectionCombo, 3, 1);
    
    // 尺寸设置 - 紧凑显示
    basicGridLayout->addWidget(createLabel("尺寸:"), 4, 0);
    m_sizeSpinBox = new QSpinBox();
    m_sizeSpinBox->setRange(100, 1000);
    m_sizeSpinBox->setValue(300);
    m_sizeSpinBox->setSuffix(" px");
    basicGridLayout->addWidget(m_sizeSpinBox, 4, 1);
    
    // 格式说明标签 - 使用滚动区域支持长文本
    m_formatInfoScrollArea = new QScrollArea();
    m_formatInfoScrollArea->setWidgetResizable(true);
    m_formatInfoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_formatInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_formatInfoScrollArea->setFrameShape(QFrame::StyledPanel);
    m_formatInfoScrollArea->setMaximumHeight(120);  // 增加最大高度
    m_formatInfoScrollArea->setMinimumHeight(60);   // 设置最小高度
    
    m_formatInfoLabel = new QLabel();
    m_formatInfoLabel->setWordWrap(true);
    m_formatInfoLabel->setAlignment(Qt::AlignTop);
    m_formatInfoLabel->setStyleSheet("QLabel { "
                                    "background-color: transparent; "
                                    "border: none; "
                                    "padding: 8px; "
                                    "font-size: 11px; "
                                    "color: #333; "
                                    "}");
    
    // 设置滚动区域的样式
    m_formatInfoScrollArea->setStyleSheet("QScrollArea { "
                                         "background-color: #f0f8ff; "
                                         "border: 1px solid #ccc; "
                                         "border-radius: 5px; "
                                         "margin: 3px 0; "
                                         "} "
                                         "QScrollBar:vertical { "
                                         "background: #f0f0f0; "
                                         "width: 12px; "
                                         "border-radius: 6px; "
                                         "} "
                                         "QScrollBar::handle:vertical { "
                                         "background: #c0c0c0; "
                                         "border-radius: 6px; "
                                         "min-height: 20px; "
                                         "} "
                                         "QScrollBar::handle:vertical:hover { "
                                         "background: #a0a0a0; "
                                         "}");
    
    m_formatInfoScrollArea->setWidget(m_formatInfoLabel);
    basicGridLayout->addWidget(m_formatInfoScrollArea, 5, 0, 1, 2);
    
    settingsLayout->addWidget(basicGroup);
    
    // Logo和文本设置组 - 合并为一个可折叠的组
    QGroupBox* advancedGroup = createGroupBox("高级设置");
    QGridLayout* advancedGridLayout = new QGridLayout(advancedGroup);
    advancedGridLayout->setSpacing(6);
    advancedGridLayout->setContentsMargins(10, 15, 10, 10);
    
    // Logo设置 - 左侧
    QLabel* logoSectionLabel = createLabel("Logo设置:");
    logoSectionLabel->setStyleSheet("font-weight: bold; color: #444;");
    advancedGridLayout->addWidget(logoSectionLabel, 0, 0, 1, 2);
    
    m_embedLogoCheckBox = new QCheckBox("嵌入Logo");
    m_embedLogoCheckBox->setEnabled(false);
    advancedGridLayout->addWidget(m_embedLogoCheckBox, 1, 0);
    
    m_selectLogoButton = new QPushButton("选择Logo");
    m_selectLogoButton->setMaximumWidth(100);
    advancedGridLayout->addWidget(m_selectLogoButton, 1, 1);
    
    m_logoPreviewLabel = new QLabel();
    m_logoPreviewLabel->setFixedSize(60, 60);
    m_logoPreviewLabel->setAlignment(Qt::AlignCenter);
    m_logoPreviewLabel->setStyleSheet(
        "border: 1px solid #cccccc; border-radius: 4px; background-color: #f5f5f5;"
    );
    m_logoPreviewLabel->setText("无Logo");
    advancedGridLayout->addWidget(m_logoPreviewLabel, 2, 0);
    
    // Logo大小控制
    QWidget* logoSizeWidget = new QWidget();
    QVBoxLayout* logoSizeLayout = new QVBoxLayout(logoSizeWidget);
    logoSizeLayout->setContentsMargins(0, 0, 0, 0);
    logoSizeLayout->setSpacing(2);
    
    logoSizeLayout->addWidget(createLabel("Logo大小:"));
    m_logoSizeSlider = new QSlider(Qt::Horizontal);
    m_logoSizeSlider->setRange(10, 30);
    m_logoSizeSlider->setValue(20);
    m_logoSizeSlider->setEnabled(false);
    logoSizeLayout->addWidget(m_logoSizeSlider);
    m_logoSizeLabel = createLabel("20%");
    m_logoSizeLabel->setAlignment(Qt::AlignCenter);
    logoSizeLayout->addWidget(m_logoSizeLabel);
    
    advancedGridLayout->addWidget(logoSizeWidget, 2, 1);
    
    // 分隔线
    QFrame* separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Sunken);
    separatorLine->setStyleSheet("color: #cccccc;");
    advancedGridLayout->addWidget(separatorLine, 3, 0, 1, 2);
    
    // 自定义文本设置 - 右侧
    QLabel* textSectionLabel = createLabel("自定义文本:");
    textSectionLabel->setStyleSheet("font-weight: bold; color: #444;");
    advancedGridLayout->addWidget(textSectionLabel, 4, 0, 1, 2);
    
    m_customTextCheckBox = new QCheckBox("添加自定义文本");
    advancedGridLayout->addWidget(m_customTextCheckBox, 5, 0, 1, 2);
    
    advancedGridLayout->addWidget(createLabel("文本内容:"), 6, 0);
    m_customTextInput = new QLineEdit();
    m_customTextInput->setPlaceholderText("请输入要显示的文本...");
    m_customTextInput->setEnabled(false);
    advancedGridLayout->addWidget(m_customTextInput, 6, 1);
    
    // 文本属性 - 紧凑布局
    advancedGridLayout->addWidget(createLabel("位置:"), 7, 0);
    m_textPositionCombo = new QComboBox();
    m_textPositionCombo->addItems({"底部", "顶部", "左侧", "右侧"});
    m_textPositionCombo->setEnabled(false);
    advancedGridLayout->addWidget(m_textPositionCombo, 7, 1);
    
    advancedGridLayout->addWidget(createLabel("大小:"), 8, 0);
    m_textSizeSpinBox = new QSpinBox();
    m_textSizeSpinBox->setRange(8, 48);
    m_textSizeSpinBox->setValue(12);
    m_textSizeSpinBox->setSuffix(" px");
    m_textSizeSpinBox->setEnabled(false);
    advancedGridLayout->addWidget(m_textSizeSpinBox, 8, 1);
    
    advancedGridLayout->addWidget(createLabel("颜色:"), 9, 0);
    m_textColorCombo = new QComboBox();
    m_textColorCombo->addItems({"黑色", "白色", "红色", "蓝色", "绿色", "灰色"});
    m_textColorCombo->setEnabled(false);
    advancedGridLayout->addWidget(m_textColorCombo, 9, 1);
    
    settingsLayout->addWidget(advancedGroup);
    
    // 自动生成选项和操作按钮
    QWidget* controlWidget = new QWidget();
    QVBoxLayout* controlLayout = createVBoxLayout(controlWidget, 5, 5);
    
    m_autoGenerateCheckBox = new QCheckBox("自动生成预览");
    m_autoGenerateCheckBox->setChecked(true);
    controlLayout->addWidget(m_autoGenerateCheckBox);
    
    // 操作按钮 - 水平排列
    QHBoxLayout* buttonLayout = createHBoxLayout(nullptr, 0, 8);
    m_generateButton = createButton("生成二维码");
    m_saveButton = createButton("保存图片");
    m_saveButton->setEnabled(false);
    buttonLayout->addWidget(m_generateButton);
    buttonLayout->addWidget(m_saveButton);
    controlLayout->addLayout(buttonLayout);
    
    settingsLayout->addWidget(controlWidget);
    settingsLayout->addStretch();
    
    // 将设置区域放入滚动区域
    scrollArea->setWidget(settingsWidget);
    
    // 右侧：预览区域
    QWidget* previewWidget = new QWidget();
    QVBoxLayout* previewLayout = createVBoxLayout(previewWidget);
    
    previewLayout->addWidget(createLabel("二维码预览:"));
    m_qrCodeLabel = new QLabel();
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setText("请输入文本并点击生成按钮");
    m_qrCodeLabel->setMinimumHeight(300);
    previewLayout->addWidget(m_qrCodeLabel);
    
    m_statusLabel = createLabel("准备就绪");
    m_statusLabel->setStyleSheet("color: #666666; font-size: 8pt;");
    previewLayout->addWidget(m_statusLabel);
    
    // 添加到主布局
    mainLayout->addWidget(scrollArea, 0, 0);
    mainLayout->addWidget(previewWidget, 0, 1);
    mainLayout->setColumnStretch(0, 0);  // 左侧固定宽度
    mainLayout->setColumnStretch(1, 1);  // 右侧可伸缩
    
    // 连接信号槽
    connectSignals();
    
    // 更新格式信息
    updateFormatInfo();
}

void GeneratorWidget::connectSignals()
{
    // 连接信号
    connect(m_generateButton, &QPushButton::clicked, this, &GeneratorWidget::onGenerateClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &GeneratorWidget::onSaveClicked);
    connect(m_selectLogoButton, &QPushButton::clicked, this, &GeneratorWidget::onSelectLogoClicked);
    connect(m_embedLogoCheckBox, &QCheckBox::toggled, this, &GeneratorWidget::onEmbedLogoToggled);
    connect(m_logoSizeSlider, &QSlider::valueChanged, this, &GeneratorWidget::onLogoSizeChanged);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &GeneratorWidget::onFormatChanged);
    
    // 自定义文本相关连接
    connect(m_customTextCheckBox, &QCheckBox::toggled, this, &GeneratorWidget::onCustomTextToggled);
    connect(m_customTextInput, &QLineEdit::textChanged, this, &GeneratorWidget::onCustomTextChanged);
    connect(m_textPositionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneratorWidget::onCustomTextChanged);
    connect(m_textSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GeneratorWidget::onCustomTextChanged);
    connect(m_textColorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GeneratorWidget::onCustomTextChanged);
    
    // 自动生成相关
    connect(m_textInput, &QLineEdit::textChanged, this, &GeneratorWidget::onAutoGenerate);
    connect(m_sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &GeneratorWidget::onAutoGenerate);
    connect(m_errorCorrectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GeneratorWidget::onAutoGenerate);
}

void GeneratorWidget::updateFormatInfo()
{
    // 初始化格式说明
    onFormatChanged();
    updateLogoControls();
}

void GeneratorWidget::updateLogoPreview()
{
    if (!m_logoPixmap.isNull()) {
        QPixmap preview = m_logoPixmap.scaled(m_logoPreviewLabel->size(), 
                                            Qt::KeepAspectRatio, 
                                            Qt::SmoothTransformation);
        m_logoPreviewLabel->setPixmap(preview);
    } else {
        m_logoPreviewLabel->clear();
        m_logoPreviewLabel->setText("无Logo");
    }
}

void GeneratorWidget::updateLogoControls()
{
    bool logoEnabled = m_embedLogoCheckBox->isChecked() && !m_logoPixmap.isNull();
    m_logoSizeSlider->setEnabled(logoEnabled);
    m_logoSizeLabel->setEnabled(logoEnabled);
}

void GeneratorWidget::onFormatChanged()
{
    // 获取当前选择的格式
    QRCodeGenerator::GenerationConfig config = getConfig();
    
    // 显示格式详细说明
    QString description = m_generator->getFormatDescription(config.format);
    m_formatInfoLabel->setText(description);
    
    // 延迟调整格式说明区域高度，确保布局完成
    QTimer::singleShot(0, this, &GeneratorWidget::adjustFormatInfoHeight);
    
    // 根据格式是否支持错误纠正来启用/禁用错误纠正选项
    bool supportsECC = m_generator->supportsErrorCorrection(config.format);
    m_errorCorrectionCombo->setEnabled(supportsECC);
    
    if (!supportsECC) {
        // 如果不支持错误纠正，显示提示
        m_errorCorrectionCombo->setToolTip("当前条码格式不支持错误纠正级别设置");
    } else {
        m_errorCorrectionCombo->setToolTip("设置条码的错误纠正级别\n"
                                          "级别越高，容错能力越强，但存储容量会减少");
    }
}

void GeneratorWidget::onCustomTextToggled(bool enabled)
{
    // 启用/禁用自定义文本相关控件
    m_customTextInput->setEnabled(enabled);
    m_textPositionCombo->setEnabled(enabled);
    m_textSizeSpinBox->setEnabled(enabled);
    m_textColorCombo->setEnabled(enabled);
    
    // 立即生成预览
    if (m_autoGenerateCheckBox->isChecked()) {
        onGenerateClicked();
    }
}

void GeneratorWidget::onCustomTextChanged()
{
    // 文本内容或设置改变时，如果开启自动生成，立即更新预览
    if (m_autoGenerateCheckBox->isChecked() && m_customTextCheckBox->isChecked()) {
        onGenerateClicked();
    }
}

void GeneratorWidget::onAutoGenerate()
{
    // 自动生成预览
    if (m_autoGenerateCheckBox->isChecked() && !m_textInput->text().isEmpty()) {
        onGenerateClicked();
    }
}

void GeneratorWidget::adjustFormatInfoHeight()
{
    if (!m_formatInfoLabel || !m_formatInfoScrollArea) {
        return;
    }
    
    // 确保标签已经完成布局
    m_formatInfoLabel->adjustSize();
    
    // 获取当前滚动区域的内容宽度（减去边框和可能的滚动条）
    int contentWidth = m_formatInfoScrollArea->viewport()->width() - 16; // 减去padding
    if (contentWidth <= 0) {
        contentWidth = m_formatInfoScrollArea->width() - 20; // 预估宽度
    }
    
    // 计算文本所需的高度
    QFontMetrics fontMetrics(m_formatInfoLabel->font());
    QRect textRect = fontMetrics.boundingRect(
        QRect(0, 0, contentWidth, 0),
        Qt::TextWordWrap,
        m_formatInfoLabel->text()
    );
    
    // 计算所需高度（加上padding和边距）
    int requiredHeight = textRect.height() + 20; // padding + margin
    
    // 设置合理的高度范围
    int minHeight = 60;
    int maxHeight = 150; // 增加最大高度
    int targetHeight = qBound(minHeight, requiredHeight, maxHeight);
    
    // 根据内容调整滚动条策略
    if (requiredHeight <= maxHeight) {
        // 内容能完全显示，不需要滚动条
        m_formatInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        targetHeight = qMax(minHeight, requiredHeight);
    } else {
        // 内容太多，需要滚动条
        m_formatInfoScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        targetHeight = maxHeight;
    }
    
    // 应用新高度
    m_formatInfoScrollArea->setMinimumHeight(targetHeight);
    m_formatInfoScrollArea->setMaximumHeight(targetHeight);
}
