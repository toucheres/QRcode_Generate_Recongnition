#include "mainwindow.h"
#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#include <QApplication>
#include <QCameraDevice>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QMediaDevices>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTextCodec>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>
// ZXing includes
#ifdef ZXING_EXPERIMENTAL_API
#include "BarcodeFormat.h"
#include "WriteBarcode.h"
#else
#include "BarcodeFormat.h"
#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#endif
#include <QSvgGenerator>
#include <iostream>
// 保存槽函数
void MainWindow::onSaveQRCode()
{
    // 检查 QLabel 是否已初始化
    if (!m_qrCodeLabel)
    {
        QMessageBox::warning(this, "错误", "QLabel 未初始化！");
        return;
    }

    // 获取 QLabel 中的图片
    const QPixmap* currentPixmap = new QPixmap{m_qrCodeLabel->pixmap()};
    if (!currentPixmap || currentPixmap->isNull())
    {
        QMessageBox::warning(this, "保存失败", "请先生成二维码！");
        return;
    }

    // 创建副本
    QPixmap pixmapToSave = *currentPixmap;

    QString fileName =
        QFileDialog::getSaveFileName(this, tr("保存二维码图片"), "",
                                     tr("PNG 图片 (*.png);;JPEG 图片 (*.jpg);;SVG 矢量图 (*.svg)"));

    if (fileName.isEmpty())
        return;

    bool success = false;
    if (fileName.endsWith(".svg", Qt::CaseInsensitive))
    {
        QSvgGenerator svgGen;
        svgGen.setFileName(fileName);
        svgGen.setSize(pixmapToSave.size());
        svgGen.setViewBox(QRect(0, 0, pixmapToSave.width(), pixmapToSave.height()));
        QPainter painter(&svgGen);
        painter.drawPixmap(0, 0, pixmapToSave);
        painter.end();
        success = true;
    }
    else
    {
        success = pixmapToSave.save(fileName);
    }

    if (success)
        QMessageBox::information(this, "保存成功", "二维码图片已保存！");
    else
        QMessageBox::warning(this, "保存失败", "保存二维码图片失败！");
}
// 保存按钮实现
void MainWindow::onGenerateQRCode()
{
    QString text = m_textInput->text().trimmed();
    if (text.isEmpty())
    {
        QMessageBox::warning(this, "警告", "请输入要生成二维码的文本！");
        return;
    }

    // 生成二维码
    QPixmap qrPixmap = generateQRCodePixmap(text);

    if (qrPixmap.isNull())
    {
        QMessageBox::warning(this, "错误", "二维码生成失败！");
        return;
    }

    // 如果启用了Logo嵌入
    if (m_embedLogoCheckBox->isChecked() && !m_logoPixmap.isNull())
    {
        int logoSize = m_logoSizeSlider->value();
        qrPixmap = embedLogoInQRCode(qrPixmap, m_logoPixmap, logoSize);
    }

    // 显示生成的二维码
    m_qrCodeLabel->setPixmap(
        qrPixmap.scaled(m_qrCodeLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_qrCodeLabel->setText("");

    // 启用保存按钮
    m_saveQRCodeButton->setEnabled(true);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_centralWidget(nullptr), m_tabWidget(nullptr),
      m_generateWidget(nullptr), m_generateTitleLabel(nullptr), m_textInput(nullptr),
      m_generateButton(nullptr), m_qrCodeLabel(nullptr), m_recognizeWidget(nullptr),
      m_recognizeTitleLabel(nullptr), m_selectFileButton(nullptr), m_urlInput(nullptr),
      m_loadUrlButton(nullptr), m_imageDisplayLabel(nullptr), m_resultTextEdit(nullptr),
      m_statusLabel(nullptr), m_cameraWidget(nullptr), m_cameraComboBox(nullptr),
      m_toggleCameraButton(nullptr), m_captureButton(nullptr), m_videoWidget(nullptr),
      m_cameraStatusLabel(nullptr), m_cameraResultTextEdit(nullptr), m_networkManager(nullptr),
      m_currentReply(nullptr), m_camera(nullptr), m_imageCapture(nullptr),
      m_captureSession(nullptr), m_recognitionTimer(nullptr), m_cameraActive(false),
      m_qrformat(nullptr), m_historyGroup(nullptr), m_historyTextEdit(nullptr),
      m_clearHistoryButton(nullptr), m_recognitionHintLabel(nullptr), m_hintTimer(nullptr),
      m_autoActionsGroup(nullptr), m_autoOpenUrlCheckBox(nullptr), m_autoCopyCheckBox(nullptr),
      m_autoOpenUrlEnabled(false), m_autoCopyEnabled(false), m_recognizeActionsGroup(nullptr),
      m_copyResultButton(nullptr), m_openUrlButton(nullptr), m_imageProcessor(new ImageProcessor),
      m_lastCaptureRequestId(-1), m_lastRecognitionTime(0), m_recognitionCount(0)
{
    qDebug() << "MainWindow constructor started";

    // 检查摄像头权限和调试信息
    checkCameraPermissions();

    setupUI();
    setWindowTitle("QR码生成识别器");
    resize(900, 750);

    qDebug() << "MainWindow constructor completed";
}

MainWindow::~MainWindow()
{
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    stopCamera();
    if (m_recognitionTimer)
    {
        m_recognitionTimer->stop();
    }
}

void MainWindow::setupUI()
{
    // 创建中央控件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);

    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #ccc; }"
        "QTabBar::tab { background: #f0f0f0; padding: 10px 20px; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #007ACC; color: white; }"
        "QTabBar::tab:hover { background: #e0e0e0; }");

    // 设置标签页
    setupGenerateMode();
    setupRecognizeMode();
    setupCameraRecognition();

    m_tabWidget->addTab(m_generateWidget, "生成模式");
    m_tabWidget->addTab(m_recognizeWidget, "识别模式");
    m_tabWidget->addTab(m_cameraWidget, "摄像头识别");

    // 连接标签页切换信号
    connect(m_tabWidget, QOverload<int>::of(&QTabWidget::currentChanged), this,
            &MainWindow::onModeChanged);
    connect(m_imageProcessor, &ImageProcessor::recognitionResult, this,
            [this](const QString& res, int _t2) { this->onAsyncRecognitionResult(res, _t2); });
    mainLayout->addWidget(m_tabWidget);

    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);

    // 初始化摄像头 - 移到setupUI的最后
    initializeCamera();
}

void MainWindow::setupGenerateMode()
{
    m_generateWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_generateWidget);

    // 标题
    m_generateTitleLabel = new QLabel("QR码生成器", m_generateWidget);
    m_generateTitleLabel->setAlignment(Qt::AlignCenter);
    m_generateTitleLabel->setStyleSheet(
        "QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");
    // 按钮
    // m_generateButton = new QPushButton("生成二维码", m_generateWidget);
    // m_saveQRCodeButton = new QPushButton("保存二维码", m_generateWidget);
    // m_saveQRCodeButton->setEnabled(false);
    // 输入框
    m_textInput = new QLineEdit(m_generateWidget);
    m_textInput->setPlaceholderText("请输入要生成二维码的文本（支持中文）...");
    m_textInput->setStyleSheet("QLineEdit { font-size: 14px; padding: 10px; margin: 10px 0; }");

    // 新增：二维码选项设置
    m_qrOptionsGroup = new QGroupBox("二维码设置");
    QVBoxLayout* optionsLayout = new QVBoxLayout(m_qrOptionsGroup);

    // 纠错级别设置
    QHBoxLayout* errorCorrectionLayout = new QHBoxLayout();
    QLabel* ecLabel = new QLabel("纠错级别:");
    m_errorCorrectionCombo = new QComboBox();
    m_errorCorrectionCombo->addItem("L - 低 (~7%)", "L");
    m_errorCorrectionCombo->addItem("M - 中 (~15%)", "M");
    m_errorCorrectionCombo->addItem("Q - 中高 (~25%)", "Q");
    m_errorCorrectionCombo->addItem("H - 高 (~30%)", "H");
    m_errorCorrectionCombo->setCurrentIndex(1); // 默认选择 M
    m_errorCorrectionCombo->setToolTip("纠错级别越高，二维码越能抵抗损坏，但密度会增加");

    errorCorrectionLayout->addWidget(ecLabel);
    errorCorrectionLayout->addWidget(m_errorCorrectionCombo);
    errorCorrectionLayout->addStretch();
    // 格式选择
    QHBoxLayout* qrformatlayout = new QHBoxLayout();

    m_qrformat = new QComboBox(m_generateWidget);

    // 添加带说明的格式选项
    m_qrformat->addItem("QR Code - 二维码", "QRCode");
    m_qrformat->addItem("Code 128 - 一维条码", "Code128");
    m_qrformat->addItem("Code 39 - 一维条码", "Code39");
    m_qrformat->addItem("Code 93 - 一维条码", "Code93");
    m_qrformat->addItem("EAN-13 - 商品条码", "EAN13");
    m_qrformat->addItem("EAN-8 - 短商品条码", "EAN8");
    m_qrformat->addItem("UPC-A - 美国商品码", "UPCA");
    m_qrformat->addItem("UPC-E - 短美国商品码", "UPCE");
    m_qrformat->addItem("Codabar - 图书馆码", "Codabar");
    m_qrformat->addItem("ITF - 交叉二五码", "ITF");
    m_qrformat->addItem("Data Matrix - 数据矩阵", "DataMatrix");
    m_qrformat->addItem("Aztec - 阿兹特克码", "Aztec");
    m_qrformat->addItem("PDF417 - 便携数据文件", "PDF417");
    m_qrformat->addItem("MaxiCode - 最大码", "MaxiCode");
    m_qrformat->addItem("Micro QR - 微型二维码", "MicroQRCode");
    m_qrformat->addItem("RM QR - 矩形微型二维码", "RMQRCode");
    m_qrformat->addItem("GS1 DataBar - 数据条", "DataBar");
    m_qrformat->addItem("GS1 DataBar Expanded - 扩展数据条", "DataBarExpanded");
    m_qrformat->addItem("GS1 DataBar Limited - 受限数据条", "DataBarLimited");
    m_qrformat->addItem("DX Film Edge - 胶片边缘码", "DXFilmEdge");

    m_qrformat->setCurrentIndex(0); // 默认选择QRCode

    // 添加工具提示
    m_qrformat->setToolTip("选择要生成的条码/二维码格式");

    qrformatlayout->addWidget(new QLabel("格式选择"));
    qrformatlayout->addWidget(m_qrformat);
    qrformatlayout->addStretch();

    // 二维码大小设置
    QHBoxLayout* sizeLayout = new QHBoxLayout();
    QLabel* sizeLabel = new QLabel("二维码大小:");
    m_qrSizeSpinBox = new QSpinBox();
    m_qrSizeSpinBox->setFixedSize(150, 25);
    m_qrSizeSpinBox->setRange(100, 800);
    m_qrSizeSpinBox->setValue(300);
    m_qrSizeSpinBox->setSuffix(" px");
    m_qrSizeSpinBox->setToolTip("设置生成的二维码图片大小");

    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(m_qrSizeSpinBox);
    sizeLayout->addStretch();

    // Logo嵌入设置
    m_embedLogoCheckBox = new QCheckBox("在二维码中心嵌入图片");
    m_embedLogoCheckBox->setToolTip("在二维码中央嵌入自定义图片，建议使用高纠错级别");

    QHBoxLayout* logoButtonLayout = new QHBoxLayout();
    m_selectLogoButton = new QPushButton("选择Logo图片");
    m_selectLogoButton->setEnabled(false);
    m_selectLogoButton->setStyleSheet(
        "QPushButton { font-size: 12px; padding: 8px 16px; background-color: #6c757d; color: "
        "white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #5a6268; "
        "} QPushButton:disabled { background-color: #cccccc; }");

    m_logoPreviewLabel = new QLabel();
    m_logoPreviewLabel->setFixedSize(60, 60);
    m_logoPreviewLabel->setStyleSheet(
        "QLabel { border: 1px solid #ccc; background-color: #f9f9f9; }");
    m_logoPreviewLabel->setAlignment(Qt::AlignCenter);
    m_logoPreviewLabel->setText("预览");
    m_logoPreviewLabel->setScaledContents(true);

    logoButtonLayout->addWidget(m_selectLogoButton);
    logoButtonLayout->addWidget(m_logoPreviewLabel);
    logoButtonLayout->addStretch();

    // Logo大小调整
    QHBoxLayout* logoSizeLayout = new QHBoxLayout();
    QLabel* logoSizeTextLabel = new QLabel("Logo大小:");
    m_logoSizeSlider = new QSlider(Qt::Horizontal);
    m_logoSizeSlider->setRange(10, 40);
    m_logoSizeSlider->setValue(20);
    m_logoSizeSlider->setEnabled(false);
    m_logoSizeLabel = new QLabel("20%");
    m_logoSizeLabel->setMinimumWidth(40);

    logoSizeLayout->addWidget(logoSizeTextLabel);
    logoSizeLayout->addWidget(m_logoSizeSlider);
    logoSizeLayout->addWidget(m_logoSizeLabel);
    logoSizeLayout->addStretch();

    // 添加到选项布局
    optionsLayout->addLayout(qrformatlayout);
    optionsLayout->addLayout(errorCorrectionLayout);
    optionsLayout->addLayout(sizeLayout);
    optionsLayout->addWidget(m_embedLogoCheckBox);
    optionsLayout->addLayout(logoButtonLayout);
    optionsLayout->addLayout(logoSizeLayout);

    // 生成按钮
    m_generateButton = new QPushButton("生成二维码", m_generateWidget);
    m_generateButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #007ACC; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #005a9e; "
        "}");
    
    //保存按钮
    m_saveQRCodeButton = new QPushButton("保存二维码", m_generateWidget);
    m_saveQRCodeButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #007ACC; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #005a9e; "
        "}");
    // 二维码显示区域
    m_qrCodeLabel = new QLabel(m_generateWidget);
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setMinimumSize(350, 350);
    m_qrCodeLabel->setStyleSheet("QLabel { border: 2px dashed #ccc; background-color: #f9f9f9; }");
    m_qrCodeLabel->setText("二维码将显示在这里");

    // 连接信号
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateQRCode);
    connect(m_textInput, &QLineEdit::returnPressed, this, &MainWindow::onGenerateQRCode);
    connect(m_embedLogoCheckBox, &QCheckBox::toggled, this, &MainWindow::onEmbedLogoChanged);
    connect(m_selectLogoButton, &QPushButton::clicked, this, &MainWindow::onSelectLogoImage);
    connect(m_logoSizeSlider, &QSlider::valueChanged, this, &MainWindow::onLogoSizeChanged);
    connect(m_saveQRCodeButton, &QPushButton::clicked, this, &MainWindow::onSaveQRCode);
    // 布局
    // layout->addWidget(m_qrformat);
    layout->addWidget(m_generateTitleLabel);
    layout->addWidget(m_textInput);
    layout->addWidget(m_qrOptionsGroup);
    layout->addWidget(m_generateButton, 0, Qt::AlignCenter);
    layout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    layout->addStretch();
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_generateButton);
    buttonLayout->addWidget(m_saveQRCodeButton);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    layout->addStretch();
}

void MainWindow::setupRecognizeMode()
{
    m_recognizeWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_recognizeWidget);

    // 标题
    m_recognizeTitleLabel = new QLabel("QR码识别器", m_recognizeWidget);
    m_recognizeTitleLabel->setAlignment(Qt::AlignCenter);
    m_recognizeTitleLabel->setStyleSheet(
        "QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");

    // 创建输入区域
    QGroupBox* inputGroup = new QGroupBox("选择图片来源");
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);

    // 文件选择
    QHBoxLayout* fileLayout = new QHBoxLayout();
    m_selectFileButton = new QPushButton("选择本地图片", m_recognizeWidget);
    m_selectFileButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; "
        "}");
    fileLayout->addWidget(m_selectFileButton);
    fileLayout->addStretch();

    // URL输入
    QHBoxLayout* urlLayout = new QHBoxLayout();
    QLabel* urlLabel = new QLabel("或输入图片URL:");
    m_urlInput = new QLineEdit(m_recognizeWidget);
    m_urlInput->setPlaceholderText("https://example.com/qrcode.png");
    m_loadUrlButton = new QPushButton("加载", m_recognizeWidget);
    m_loadUrlButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 8px 16px; background-color: #17a2b8; color: "
        "white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #138496; "
        "}");

    m_loadUrlButton->setEnabled(false);

    urlLayout->addWidget(urlLabel);
    urlLayout->addWidget(m_urlInput, 1);
    urlLayout->addWidget(m_loadUrlButton);

    inputLayout->addLayout(fileLayout);
    inputLayout->addLayout(urlLayout);

    // 创建显示区域
    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    // 图片显示
    QGroupBox* imageGroup = new QGroupBox("图片预览");
    QVBoxLayout* imageLayout = new QVBoxLayout(imageGroup);
    m_imageDisplayLabel = new QLabel();
    m_imageDisplayLabel->setAlignment(Qt::AlignCenter);
    m_imageDisplayLabel->setMinimumSize(300, 300);
    m_imageDisplayLabel->setStyleSheet(
        "QLabel { border: 2px dashed #ccc; background-color: #f9f9f9; }");
    m_imageDisplayLabel->setText("选择图片后将在此显示");
    m_imageDisplayLabel->setScaledContents(true);
    imageLayout->addWidget(m_imageDisplayLabel);

    // 右侧区域：结果显示和操作按钮
    QWidget* rightWidget = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);

    // 结果显示
    QGroupBox* resultGroup = new QGroupBox("识别结果");
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
    m_resultTextEdit = new QTextEdit();
    m_resultTextEdit->setPlaceholderText("识别结果将显示在这里...");
    m_resultTextEdit->setStyleSheet("QTextEdit { font-size: 14px; padding: 10px; }");
    m_resultTextEdit->setReadOnly(true);
    resultLayout->addWidget(m_resultTextEdit);

    // 新增：操作按钮区域
    m_recognizeActionsGroup = new QGroupBox("操作选项");
    QVBoxLayout* actionsLayout = new QVBoxLayout(m_recognizeActionsGroup);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_copyResultButton = new QPushButton("复制内容");
    m_copyResultButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #17a2b8; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #138496; "
        "} QPushButton:disabled { background-color: #6c757d; }");
    m_copyResultButton->setToolTip("将识别结果复制到剪贴板");
    m_copyResultButton->setEnabled(false);

    m_openUrlButton = new QPushButton("打开网址");
    m_openUrlButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; "
        "} QPushButton:disabled { background-color: #6c757d; }");
    m_openUrlButton->setToolTip("如果识别结果是网址，在浏览器中打开");
    m_openUrlButton->setEnabled(false);

    buttonLayout->addWidget(m_copyResultButton);
    buttonLayout->addWidget(m_openUrlButton);
    buttonLayout->addStretch();

    actionsLayout->addLayout(buttonLayout);

    // 添加提示文本
    QLabel* hintLabel = new QLabel("提示：识别成功后可以复制内容或打开网址");
    hintLabel->setStyleSheet("QLabel { color: #666; font-size: 12px; font-style: italic; }");
    actionsLayout->addWidget(hintLabel);

    rightLayout->addWidget(resultGroup);
    rightLayout->addWidget(m_recognizeActionsGroup);
    rightLayout->addStretch();

    splitter->addWidget(imageGroup);
    splitter->addWidget(rightWidget);
    splitter->setSizes({350, 350});

    // 状态标签
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");

    // 连接信号
    connect(m_selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectImageFile);
    connect(m_urlInput, &QLineEdit::textChanged, this, &MainWindow::onUrlInputChanged);
    connect(m_loadUrlButton, &QPushButton::clicked, this, &MainWindow::onLoadFromUrl);
    connect(m_urlInput, &QLineEdit::returnPressed, this, &MainWindow::onLoadFromUrl);
    connect(m_copyResultButton, &QPushButton::clicked, this, &MainWindow::onCopyRecognizedText);
    connect(m_openUrlButton, &QPushButton::clicked, this, &MainWindow::onOpenRecognizedUrl);

    // 布局
    mainLayout->addWidget(m_recognizeTitleLabel);
    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(m_statusLabel);
}

void MainWindow::setupCameraRecognition()
{
    m_cameraWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_cameraWidget);

    // 标题
    QLabel* titleLabel = new QLabel("摄像头QR码识别", m_cameraWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");

    // 控制区域
    QGroupBox* controlGroup = new QGroupBox("摄像头控制");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);

    // 摄像头选择
    QHBoxLayout* cameraSelectLayout = new QHBoxLayout();
    QLabel* cameraLabel = new QLabel("选择摄像头:");
    m_cameraComboBox = new QComboBox();

    // 添加刷新按钮
    QPushButton* refreshButton = new QPushButton("刷新");
    refreshButton->setStyleSheet(
        "QPushButton { font-size: 12px; padding: 5px 10px; background-color: #6c757d; color: "
        "white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #5a6268; "
        "}");

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::updateCameraList);

    cameraSelectLayout->addWidget(cameraLabel);
    cameraSelectLayout->addWidget(m_cameraComboBox, 1);
    cameraSelectLayout->addWidget(refreshButton);
    cameraSelectLayout->addStretch();

    // 控制按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_toggleCameraButton = new QPushButton("启动摄像头");
    m_toggleCameraButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; "
        "}");

    m_captureButton = new QPushButton("拍照识别");
    m_captureButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #007ACC; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #005a9e; "
        "}");

    m_captureButton->setEnabled(false);

    buttonLayout->addWidget(m_toggleCameraButton);
    buttonLayout->addWidget(m_captureButton);
    buttonLayout->addStretch();

    controlLayout->addLayout(cameraSelectLayout);
    controlLayout->addLayout(buttonLayout);

    // 新增：自动功能选项
    m_autoActionsGroup = new QGroupBox("自动功能");
    QVBoxLayout* autoLayout = new QVBoxLayout(m_autoActionsGroup);

    m_autoOpenUrlCheckBox = new QCheckBox("识别到网址时自动跳转");
    m_autoOpenUrlCheckBox->setToolTip("当识别到的内容是有效网址时，自动在浏览器中打开");
    m_autoOpenUrlCheckBox->setStyleSheet("QCheckBox { font-size: 12px; padding: 3px; }");

    m_autoCopyCheckBox = new QCheckBox("自动复制识别内容到剪贴板");
    m_autoCopyCheckBox->setToolTip("每次成功识别二维码后，自动将内容复制到系统剪贴板");
    m_autoCopyCheckBox->setStyleSheet("QCheckBox { font-size: 12px; padding: 3px; }");

    autoLayout->addWidget(m_autoOpenUrlCheckBox);
    autoLayout->addWidget(m_autoCopyCheckBox);

    controlLayout->addWidget(m_autoActionsGroup);

    // 新增：识别提示标签
    m_recognitionHintLabel = new QLabel();
    m_recognitionHintLabel->setAlignment(Qt::AlignCenter);
    m_recognitionHintLabel->setStyleSheet(
        "QLabel { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; "
        "border-radius: 5px; padding: 8px; margin: 5px; }");
    m_recognitionHintLabel->hide();
    controlLayout->addWidget(m_recognitionHintLabel);

    // 创建水平分割器
    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    // 视频显示区域
    QGroupBox* videoGroup = new QGroupBox("摄像头画面");
    QVBoxLayout* videoLayout = new QVBoxLayout(videoGroup);
    m_videoWidget = new QVideoWidget();
    m_videoWidget->setMinimumSize(400, 300);
    m_videoWidget->setStyleSheet(
        "QVideoWidget { border: 2px solid #ccc; background-color: #000; }");
    videoLayout->addWidget(m_videoWidget);

    // 右侧区域：结果显示和历史记录
    QWidget* rightWidget = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);

    // 实时识别结果显示区域
    QGroupBox* resultGroup = new QGroupBox("实时识别结果");
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
    m_cameraResultTextEdit = new QTextEdit();
    m_cameraResultTextEdit->setPlaceholderText(
        "实时识别结果将显示在这里...\n启动摄像头后会自动尝试识别画面中的二维码");
    m_cameraResultTextEdit->setStyleSheet("QTextEdit { font-size: 14px; padding: 10px; }");
    m_cameraResultTextEdit->setMaximumHeight(150);
    resultLayout->addWidget(m_cameraResultTextEdit);

    // 新增：历史记录区域
    m_historyGroup = new QGroupBox("识别历史记录");
    QVBoxLayout* historyLayout = new QVBoxLayout(m_historyGroup);

    QHBoxLayout* historyHeaderLayout = new QHBoxLayout();
    QLabel* historyLabel = new QLabel("历史记录 (自动去重):");
    m_clearHistoryButton = new QPushButton("清空记录");
    m_clearHistoryButton->setStyleSheet(
        "QPushButton { font-size: 12px; padding: 5px 10px; background-color: #dc3545; color: "
        "white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #c82333; "
        "}");

    historyHeaderLayout->addWidget(historyLabel);
    historyHeaderLayout->addStretch();
    historyHeaderLayout->addWidget(m_clearHistoryButton);

    m_historyTextEdit = new QTextEdit();
    m_historyTextEdit->setPlaceholderText("识别到的二维码内容将按时间顺序显示在这里...");
    m_historyTextEdit->setStyleSheet("QTextEdit { font-size: 12px; padding: 8px; }");
    m_historyTextEdit->setReadOnly(true);

    historyLayout->addLayout(historyHeaderLayout);
    historyLayout->addWidget(m_historyTextEdit);

    rightLayout->addWidget(resultGroup);
    rightLayout->addWidget(m_historyGroup);

    splitter->addWidget(videoGroup);
    splitter->addWidget(rightWidget);
    splitter->setSizes({450, 400});

    // 状态标签
    m_cameraStatusLabel = new QLabel("摄像头未启动");
    m_cameraStatusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");

    // 连接信号
    connect(m_toggleCameraButton, &QPushButton::clicked, this, &MainWindow::onToggleCamera);
    connect(m_captureButton, &QPushButton::clicked, this, &MainWindow::onCaptureImage);
    connect(m_cameraComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::onCameraChanged);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, &MainWindow::onClearHistory);
    connect(m_autoOpenUrlCheckBox, &QCheckBox::toggled, this, &MainWindow::onAutoOpenUrlChanged);
    connect(m_autoCopyCheckBox, &QCheckBox::toggled, this, &MainWindow::onAutoCopyChanged);

    // 布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(m_cameraStatusLabel);
}

void MainWindow::initializeCamera()
{
    qDebug() << "Initializing camera...";

    // 初始化定时器用于实时识别
    m_recognitionTimer = new QTimer(this);
    m_recognitionTimer->setInterval(1000); // 每秒识别一次
    connect(m_recognitionTimer, &QTimer::timeout, this, &MainWindow::onRecognitionTimerTimeout);

    // 新增：初始化提示定时器
    m_hintTimer = new QTimer(this);
    m_hintTimer->setSingleShot(true);
    m_hintTimer->setInterval(3000); // 3秒后隐藏提示
    connect(m_hintTimer, &QTimer::timeout, this, [this]() { m_recognitionHintLabel->hide(); });

    // 创建捕获会话
    m_captureSession = new QMediaCaptureSession(this);
    m_imageCapture = new QImageCapture(this);

    m_captureSession->setImageCapture(m_imageCapture);
    m_captureSession->setVideoOutput(m_videoWidget);

    // 连接图像捕获信号
    connect(m_imageCapture, &QImageCapture::imageCaptured, this, &MainWindow::onImageCaptured);

    // 延迟更新摄像头列表，确保系统完全初始化
    QTimer::singleShot(500, this, &MainWindow::updateCameraList);
}

void MainWindow::updateCameraList()
{
    qDebug() << "Updating camera list...";

    m_cameraComboBox->clear();

    // 获取可用摄像头设备
    m_availableCameras = QMediaDevices::videoInputs();

    qDebug() << "Found" << m_availableCameras.size() << "camera devices";

    if (m_availableCameras.isEmpty())
    {
        m_cameraComboBox->addItem("未找到摄像头");
        m_toggleCameraButton->setEnabled(false);
        m_cameraStatusLabel->setText("未检测到摄像头设备");

        // 检查权限和驱动问题
        qDebug() << "No cameras found. Possible issues:";
        qDebug() << "1. Camera permission not granted";
        qDebug() << "2. Camera driver not installed";
        qDebug() << "3. Camera already in use by another application";

        // 显示详细错误信息给用户
        QTimer::singleShot(100, this,
                           [this]()
                           {
                               QString errorMsg = "未找到可用摄像头，可能的原因：\n\n";
                               errorMsg += "1. 摄像头权限未授予\n";
                               errorMsg += "2. 摄像头驱动未正确安装\n";
                               errorMsg += "3. 摄像头正被其他应用程序使用\n";
                               errorMsg += "4. 系统中没有连接摄像头设备\n\n";
                               errorMsg += "请检查系统设置并重试。";

                               QMessageBox::warning(this, "摄像头检测", errorMsg);
                           });

        return;
    }

    // 添加摄像头到列表
    for (int i = 0; i < m_availableCameras.size(); ++i)
    {
        const QCameraDevice& cameraDevice = m_availableCameras.at(i);
        QString deviceInfo =
            QString("%1 (%2)").arg(cameraDevice.description()).arg(cameraDevice.id());

        m_cameraComboBox->addItem(deviceInfo);

        qDebug() << "Camera" << i << ":"
                 << "Description:" << cameraDevice.description() << "ID:" << cameraDevice.id()
                 << "Default:" << cameraDevice.isDefault();
    }

    // 选择默认摄像头
    for (int i = 0; i < m_availableCameras.size(); ++i)
    {
        if (m_availableCameras.at(i).isDefault())
        {
            m_cameraComboBox->setCurrentIndex(i);
            break;
        }
    }

    m_toggleCameraButton->setEnabled(true);
    m_cameraStatusLabel->setText(QString("检测到 %1 个摄像头设备").arg(m_availableCameras.size()));
}

void MainWindow::startCamera()
{
    qDebug() << "Starting camera...";

    if (m_availableCameras.isEmpty())
    {
        QMessageBox::warning(this, "错误", "未找到可用的摄像头！\n请检查摄像头连接和权限设置。");
        return;
    }

    try
    {
        // 停止之前的摄像头
        if (m_camera)
        {
            m_camera->stop();
            m_camera->deleteLater();
            m_camera = nullptr;
        }

        int selectedIndex = m_cameraComboBox->currentIndex();
        if (selectedIndex < 0 || selectedIndex >= m_availableCameras.size())
        {
            QMessageBox::warning(this, "错误", "无效的摄像头选择！");
            return;
        }

        const QCameraDevice& selectedCamera = m_availableCameras.at(selectedIndex);
        qDebug() << "Selected camera:" << selectedCamera.description();

        // 创建摄像头
        m_camera = new QCamera(selectedCamera, this);

        // 连接错误信号
        connect(m_camera, &QCamera::errorOccurred, this, &MainWindow::onCameraError);

        // 连接状态变化信号
        connect(m_camera, &QCamera::activeChanged, this,
                [this](bool active)
                {
                    qDebug() << "Camera active state changed:" << active;
                    if (active)
                    {
                        m_cameraStatusLabel->setText("摄像头已启动 - 实时识别中...");

                        // 启动异步图像处理器
                        m_imageProcessor->start();
                    }
                });

        // 设置摄像头到捕获会话
        m_captureSession->setCamera(m_camera);

        // 检查摄像头是否可用
        if (!m_camera->isAvailable())
        {
            QMessageBox::warning(this, "错误",
                                 "选择的摄像头当前不可用！\n可能正被其他应用程序使用。");
            return;
        }

        // 启动摄像头
        m_camera->start();

        // 等待摄像头启动
        QTimer::singleShot(
            1000, this,
            [this]()
            {
                if (m_camera && m_camera->isActive())
                {
                    m_cameraActive = true;
                    m_toggleCameraButton->setText("停止摄像头");
                    m_toggleCameraButton->setStyleSheet(
                        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: "
                        "#dc3545; color: white; border: none; border-radius: 5px; } "
                        "QPushButton:hover { background-color: #c82333; }");
                    m_captureButton->setEnabled(true);
                    m_cameraComboBox->setEnabled(false);

                    // 启动实时识别定时器 - 降低频率减少CPU占用
                    m_recognitionTimer->start();

                    // 设置处理间隔为1.5秒，避免过于频繁的识别
                    m_imageProcessor->setProcessingInterval(1500);

                    qDebug() << "Camera started successfully";
                }
                else
                {
                    QMessageBox::warning(this, "错误",
                                         "摄像头启动失败！\n请检查摄像头权限和驱动程序。");
                }
            });
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "错误", QString("启动摄像头失败：%1").arg(e.what()));
        qDebug() << "Camera start exception:" << e.what();
    }
}

// void MainWindow::stopCamera()
// {
//     // 停止异步处理器
//     if (m_imageProcessor)
//     {
//         m_imageProcessor->stop();
//     }

//     if (m_camera)
//     {
//         m_camera->stop();
//         m_camera->deleteLater();
//         m_camera = nullptr;
//     }

//     m_recognitionTimer->stop();

//     m_cameraActive = false;
//     m_toggleCameraButton->setText("启动摄像头");
//     m_toggleCameraButton->setStyleSheet(
//         "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: "
//         "white; border: none; border-radius: 5px; } QPushButton:hover { background-color:
//         #218838; "
//         "}");
//     m_captureButton->setEnabled(false);
//     m_cameraComboBox->setEnabled(true);
//     m_cameraStatusLabel->setText("摄像头已停止");
// }

void MainWindow::onCaptureImage()
{
    if (m_imageCapture && m_cameraActive)
    {
        m_imageCapture->capture();
    }
}

void MainWindow::onCameraChanged(int index)
{
    if (m_cameraActive)
    {
        // 如果摄像头正在运行，先停止再重新启动
        stopCamera();
        startCamera();
    }
}

// void MainWindow::onImageCaptured(int id, const QImage& image)
// {
//     Q_UNUSED(id)

//     if (image.isNull())
//     {
//         return;
//     }

//     QString result = recognizeQRCodeFromImage(image);

//     // 更新实时结果显示
//     m_cameraResultTextEdit->clear();
//     if (result.startsWith("未找到") || result.startsWith("识别错误"))
//     {
//         m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #dc3545; }");
//         m_cameraResultTextEdit->setPlainText(QString("拍照识别结果：%1").arg(result));
//     }
//     else
//     {
//         m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #28a745; font-weight: bold;
//         }");
//         m_cameraResultTextEdit->setPlainText(QString("拍照识别成功！\n内容：%1").arg(result));

//         // 新增：添加到历史记录
//         addToHistory(result);

//         // 新增：处理自动操作
//         handleAutoActions(result);

//         // 新增：显示识别提示
//         showRecognitionHint(result);
//     }
// }

void MainWindow::onCameraError(QCamera::Error error)
{
    QString errorString;
    switch (error)
    {
    case QCamera::NoError:
        return;
    case QCamera::CameraError:
        errorString = "摄像头硬件错误 - 请检查摄像头连接";
        break;
    default:
        errorString = QString("摄像头错误 (代码: %1)").arg(static_cast<int>(error));
        break;
    }

    qDebug() << "Camera error occurred:" << errorString;
    QMessageBox::warning(this, "摄像头错误", errorString);
    stopCamera();
}

// void MainWindow::onRecognitionTimerTimeout()
// {
//     // 实时识别（每秒触发一次）
//     if (m_cameraActive && m_imageCapture)
//     {
//         recognizeFromVideoFrame();
//     }
// }

void MainWindow::recognizeFromVideoFrame()
{
    // 这里可以实现从视频流中提取帧进行识别
    // 由于Qt6的VideoFrame API限制，我们使用定时拍照的方式
    if (m_imageCapture && m_cameraActive)
    {
        // 静默捕获用于实时识别
        m_imageCapture->capture();
    }
}

QPixmap MainWindow::zxingMatrixToQPixmap(const ZXing::BitMatrix& matrix)
{
    int width = matrix.width();
    int height = matrix.height();

    QImage image(width, height, QImage::Format_RGB32);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            QRgb color = matrix.get(x, y) ? qRgb(0, 0, 0) : qRgb(255, 255, 255);
            image.setPixel(x, y, color);
        }
    }

    return QPixmap::fromImage(image);
}

void MainWindow::onSelectLogoImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择Logo图片", "",
                                                    "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (!fileName.isEmpty())
    {
        QPixmap logoPixmap(fileName);
        if (logoPixmap.isNull())
        {
            QMessageBox::warning(this, "错误", "无法加载Logo图片文件！");
            return;
        }

        m_logoPixmap = logoPixmap;

        // 更新预览
        QPixmap preview = logoPixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_logoPreviewLabel->setPixmap(preview);

        // 启用大小滑块
        m_logoSizeSlider->setEnabled(true);

        QMessageBox::information(this, "成功",
                                 "Logo图片加载成功！\n建议使用较高的纠错级别以确保二维码可读性。");
    }
}

void MainWindow::onEmbedLogoChanged(bool enabled)
{
    m_selectLogoButton->setEnabled(enabled);
    m_logoSizeSlider->setEnabled(enabled && !m_logoPixmap.isNull());

    if (enabled)
    {
        // 当启用Logo嵌入时，建议使用更高的纠错级别
        if (m_errorCorrectionCombo->currentIndex() < 2)
        {
            QMessageBox::information(
                this, "提示", "启用Logo嵌入时建议使用Q或H级别的纠错等级以确保二维码的可读性。");
            m_errorCorrectionCombo->setCurrentIndex(2); // 设置为Q级别
        }
    }
}

void MainWindow::onLogoSizeChanged(int size)
{
    m_logoSizeLabel->setText(QString("%1%").arg(size));
}

QPixmap MainWindow::generateQRCodePixmap(const QString& text)
{
    try
    {
        // 确保使用UTF-8编码
        QByteArray utf8Data = text.toUtf8();
        std::string stdText = utf8Data.toStdString();

        // 获取设置参数
        QString ecLevel = m_errorCorrectionCombo->currentData().toString();
        int qrSize = m_qrSizeSpinBox->value();

        qDebug() << "Generating QR code for UTF-8 text:" << QString::fromUtf8(utf8Data);
        qDebug() << "std::string size:" << stdText.size();
        qDebug() << "Error correction level:" << ecLevel;
        qDebug() << "Target size:" << qrSize;

#ifdef ZXING_EXPERIMENTAL_API
        // 使用实验性API
        qDebug() << "Using experimental API path";

        try
        {
            qDebug() << "Creating CreatorOptions with QRCode format...";
            // ZXing::CreatorOptions options(ZXing::BarcodeFormat::QRCode);
            ZXing::CreatorOptions options(
                [this]()
                {
                    QMap<QString, ZXing::BarcodeFormat> trans{
                        {"QRCode", ZXing::BarcodeFormat::QRCode},
                        {"Code128", ZXing::BarcodeFormat::Code128},
                        {"Code39", ZXing::BarcodeFormat::Code39},
                        {"Code93", ZXing::BarcodeFormat::Code93},
                        {"EAN13", ZXing::BarcodeFormat::EAN13},
                        {"EAN8", ZXing::BarcodeFormat::EAN8},
                        {"UPCA", ZXing::BarcodeFormat::UPCA},
                        {"UPCE", ZXing::BarcodeFormat::UPCE},
                        {"Codabar", ZXing::BarcodeFormat::Codabar},
                        {"ITF", ZXing::BarcodeFormat::ITF},
                        {"DataMatrix", ZXing::BarcodeFormat::DataMatrix},
                        {"Aztec", ZXing::BarcodeFormat::Aztec},
                        {"PDF417", ZXing::BarcodeFormat::PDF417},
                        {"MaxiCode", ZXing::BarcodeFormat::MaxiCode},
                        {"Micro QR", ZXing::BarcodeFormat::MicroQRCode},
                        {"RM QR", ZXing::BarcodeFormat::RMQRCode},
                        {"GS1 DataBar", ZXing::BarcodeFormat::DataBar},
                        {"GS1 DataBar Expanded", ZXing::BarcodeFormat::DataBarExpanded},
                        {"GS1 DataBar Limited", ZXing::BarcodeFormat::DataBarLimited},
                        {"DX Film Edge", ZXing::BarcodeFormat::DXFilmEdge}};
                    return trans[QString{this->m_qrformat->currentData().toString()}];
                }());
            qDebug() << "CreatorOptions created successfully";

            // 修复：将字母转换为数字字符串
            std::string ecLevelStd;
            if (ecLevel == "L")
            {
                ecLevelStd = "0";
            }
            else if (ecLevel == "M")
            {
                ecLevelStd = "1";
            }
            else if (ecLevel == "Q")
            {
                ecLevelStd = "2";
            }
            else if (ecLevel == "H")
            {
                ecLevelStd = "3";
            }
            else
            {
                ecLevelStd = "1"; // 默认M级别
            }

            qDebug() << "Converting ecLevel from" << ecLevel
                     << "to numeric string:" << QString::fromStdString(ecLevelStd);
            qDebug() << "About to call options.ecLevel()...";

            options.ecLevel(ecLevelStd); // 设置错误纠正级别（使用数字字符串）
            qDebug() << "ecLevel set successfully";

            qDebug() << "About to create barcode from text...";
            qDebug() << "Text for barcode creation:" << QString::fromStdString(stdText);

            auto barcode = ZXing::CreateBarcodeFromText(stdText, options);
            qDebug() << "CreateBarcodeFromText completed";

            if (!barcode.isValid())
            {
                qDebug() << "Failed to create barcode - barcode is not valid";
                qDebug() << "Falling back to legacy API...";
                // 直接跳转到传统API
                throw std::runtime_error("Experimental API failed, using fallback");
            }

            qDebug() << "Barcode created successfully, creating writer options...";
            ZXing::WriterOptions writerOptions;
            writerOptions.sizeHint(qrSize).withQuietZones(true);
            qDebug() << "Writer options configured";

            qDebug() << "Writing barcode to image...";
            auto image = ZXing::WriteBarcodeToImage(barcode, writerOptions);
            qDebug() << "Image created successfully, size:" << image.width() << "x"
                     << image.height();

            // 将ZXing Image转换为QPixmap
            QImage qImage(image.data(), image.width(), image.height(), QImage::Format_Grayscale8);
            qDebug() << "QImage created, size:" << qImage.width() << "x" << qImage.height();

            return QPixmap::fromImage(qImage);
        }
        catch (const std::exception& expApiError)
        {
            qDebug() << "Experimental API error:" << expApiError.what();
            qDebug() << "Falling back to legacy API...";
            // 继续执行到传统API部分
        }

#else
        qDebug() << "Experimental API not available, using legacy API";
#endif

        // 使用传统API（作为备用或主要方法）
        qDebug() << "Using legacy API path";
        ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);
        writer.setMargin(2);

        // 修复错误纠正级别设置 - 不使用stoi
        int ecLevelInt = 1; // 默认M级别
        if (ecLevel == "L")
        {
            ecLevelInt = 0;
        }
        else if (ecLevel == "M")
        {
            ecLevelInt = 1;
        }
        else if (ecLevel == "Q")
        {
            ecLevelInt = 2;
        }
        else if (ecLevel == "H")
        {
            ecLevelInt = 3;
        }
        else
        {
            // 如果是未知值，默认使用M级别
            qDebug() << "Unknown error correction level:" << ecLevel << ", using M level";
            ecLevelInt = 1;
        }

        qDebug() << "Setting ECC level to:" << ecLevelInt;
        writer.setEccLevel(ecLevelInt);

        // 设置编码为UTF-8以支持中文字符
        writer.setEncoding(ZXing::CharacterSet::UTF8);

        // 创建二维码矩阵
        qDebug() << "Encoding text to BitMatrix...";
        auto bitMatrix = writer.encode(stdText, qrSize, qrSize);
        qDebug() << "BitMatrix size:" << bitMatrix.width() << "x" << bitMatrix.height();

        return zxingMatrixToQPixmap(bitMatrix);
    }
    catch (const std::invalid_argument& e)
    {
        qDebug() << "Invalid argument error:" << e.what();
        qDebug() << "Stack trace or detailed error info would be helpful here";
        // 尝试使用默认设置重新生成
        try
        {
            qDebug() << "Attempting recovery with minimal parameters...";
            ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);
            writer.setMargin(2);
            writer.setEccLevel(1); // M级别
            writer.setEncoding(ZXing::CharacterSet::UTF8);

            QByteArray utf8Data = text.toUtf8();
            std::string stdText = utf8Data.toStdString();
            int qrSize = m_qrSizeSpinBox->value();

            auto bitMatrix = writer.encode(stdText, qrSize, qrSize);
            qDebug() << "Recovery successful";
            return zxingMatrixToQPixmap(bitMatrix);
        }
        catch (const std::exception& recoveryError)
        {
            qDebug() << "Recovery also failed:" << recoveryError.what();
            return QPixmap();
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "General error generating QR code:" << e.what();
        qDebug() << "Error type:" << typeid(e).name();
        return QPixmap();
    }
}

QPixmap MainWindow::embedLogoInQRCode(const QPixmap& qrCode, const QPixmap& logo,
                                      int logoSizePercent)
{
    if (qrCode.isNull() || logo.isNull())
    {
        return qrCode;
    }

    // 计算Logo大小
    int qrSize = qrCode.width();
    int logoSize = qrSize * logoSizePercent / 100;

    // 确保Logo大小是偶数，避免位置偏移
    if (logoSize % 2 != 0)
    {
        logoSize--;
    }

    // 缩放Logo
    QPixmap scaledLogo =
        logo.scaled(logoSize, logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建结果图像
    QPixmap result = qrCode.copy();

    // 在Logo周围添加白色边框以提高对比度
    int borderSize = logoSize / 10; // 边框大小为Logo的10%
    QPixmap logoWithBorder(scaledLogo.width() + 2 * borderSize,
                           scaledLogo.height() + 2 * borderSize);
    logoWithBorder.fill(Qt::white);

    // 将Logo绘制到带边框的图像中央
    QPainter borderPainter(&logoWithBorder);
    borderPainter.setRenderHint(QPainter::Antialiasing);

    int logoX = (logoWithBorder.width() - scaledLogo.width()) / 2;
    int logoY = (logoWithBorder.height() - scaledLogo.height()) / 2;
    borderPainter.drawPixmap(logoX, logoY, scaledLogo);
    borderPainter.end();

    // 将Logo绘制到二维码中央
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    int x = (qrSize - logoWithBorder.width()) / 2;
    int y = (qrSize - logoWithBorder.height()) / 2;

    painter.drawPixmap(x, y, logoWithBorder);
    painter.end();

    return result;
}

void MainWindow::onSelectImageFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "选择二维码图片", "", "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)");

    if (!fileName.isEmpty())
    {
        QPixmap pixmap(fileName);
        if (pixmap.isNull())
        {
            QMessageBox::warning(this, "错误", "无法加载图片文件！");
            return;
        }

        // 存储原始图像
        m_originalImagePixmap = pixmap;
        
        // 使用新的检测函数获取位置信息
        m_currentDetectionResult = detectQRCodeFromImage(pixmap.toImage());
        
        if (m_currentDetectionResult.isValid)
        {
            // 绘制轮廓并显示
            QPixmap pixmapWithContour = drawQRCodeContour(pixmap, m_currentDetectionResult.position);
            displaySelectedImage(pixmapWithContour);
            displayRecognitionResult(m_currentDetectionResult.text);
            updateRecognizeActionButtons(m_currentDetectionResult.text);
        }
        else
        {
            // 如果检测失败，显示原图并尝试传统识别
            displaySelectedImage(pixmap);
            QString result = recognizeQRCodeFromPixmap(pixmap);
            displayRecognitionResult(result);
            updateRecognizeActionButtons(result);
        }
    }
}

void MainWindow::onLoadFromUrl()
{
    QString urlText = m_urlInput->text().trimmed();
    if (urlText.isEmpty())
    {
        return;
    }

    QUrl url(urlText);
    if (!url.isValid())
    {
        QMessageBox::warning(this, "错误", "请输入有效的URL地址！");
        return;
    }

    // 取消之前的请求
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_statusLabel->setText("正在加载图片...");
    m_loadUrlButton->setEnabled(false);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    m_currentReply = m_networkManager->get(request);

    connect(m_currentReply, &QNetworkReply::finished, this, &MainWindow::onNetworkReplyFinished);
}

void MainWindow::onUrlInputChanged()
{
    bool hasText = !m_urlInput->text().trimmed().isEmpty();
    m_loadUrlButton->setEnabled(hasText);
}

void MainWindow::onNetworkReplyFinished()
{
    if (!m_currentReply)
    {
        return;
    }

    m_loadUrlButton->setEnabled(true);

    if (m_currentReply->error() != QNetworkReply::NoError)
    {
        m_statusLabel->setText("加载失败");
        QMessageBox::warning(this, "网络错误",
                             QString("无法加载图片：%1").arg(m_currentReply->errorString()));
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    QByteArray imageData = m_currentReply->readAll();
    QPixmap pixmap;

    if (!pixmap.loadFromData(imageData))
    {
        m_statusLabel->setText("图片格式错误");
        QMessageBox::warning(this, "错误", "下载的文件不是有效的图片格式！");
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    m_statusLabel->setText("加载完成");
    
    // 存储原始图像
    m_originalImagePixmap = pixmap;
    
    // 使用新的检测函数获取位置信息
    m_currentDetectionResult = detectQRCodeFromImage(pixmap.toImage());
    
    if (m_currentDetectionResult.isValid)
    {
        // 绘制轮廓并显示
        QPixmap pixmapWithContour = drawQRCodeContour(pixmap, m_currentDetectionResult.position);
        displaySelectedImage(pixmapWithContour);
        displayRecognitionResult(m_currentDetectionResult.text);
        updateRecognizeActionButtons(m_currentDetectionResult.text);
    }
    else
    {
        // 如果检测失败，显示原图并尝试传统识别
        displaySelectedImage(pixmap);
        QString result = recognizeQRCodeFromPixmap(pixmap);
        displayRecognitionResult(result);
        updateRecognizeActionButtons(result);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void MainWindow::onModeChanged(int index)
{
    index_ = index;
    qDebug() << "Mode changed to:"
             << (index == 0 ? "Generate" : (index == 1 ? "Recognize" : "Camera"));

    // 当切换到其他模式时，停止摄像头
    if (index != 2 && m_cameraActive)
    {
        stopCamera();
    }
}

QString MainWindow::recognizeQRCodeFromPixmap(const QPixmap& pixmap)
{
    return recognizeQRCodeFromImage(pixmap.toImage());
}

QString MainWindow::recognizeQRCodeFromImage(const QImage& image)
{
    try
    {
        // 转换为ZXing可以处理的格式
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

        ZXing::ImageView imageView(rgbImage.bits(), rgbImage.width(), rgbImage.height(),
                                   ZXing::ImageFormat::RGB, rgbImage.bytesPerLine());

        // 定义常用的条码格式列表，按使用频率排序
        std::vector<ZXing::BarcodeFormat> formatList = {
            ZXing::BarcodeFormat::QRCode,      // 最常用的二维码
            ZXing::BarcodeFormat::Code128,     // 常用一维码
            ZXing::BarcodeFormat::Code39,      // 常用一维码
            ZXing::BarcodeFormat::EAN13,       // 商品条码
            ZXing::BarcodeFormat::DataMatrix,  // 数据矩阵
            ZXing::BarcodeFormat::Aztec,       // 阿兹特克码
            ZXing::BarcodeFormat::PDF417,      // PDF417
            ZXing::BarcodeFormat::Code93,      // Code93
            ZXing::BarcodeFormat::EAN8,        // EAN8
            ZXing::BarcodeFormat::UPCA,        // UPC-A
            ZXing::BarcodeFormat::UPCE,        // UPC-E
            ZXing::BarcodeFormat::ITF,         // ITF
            ZXing::BarcodeFormat::Codabar,     // Codabar
            ZXing::BarcodeFormat::MicroQRCode, // 微型QR码
            ZXing::BarcodeFormat::MaxiCode,    // MaxiCode
        };

        // 定义不同的识别配置，从宽松到严格
        std::vector<std::function<void(ZXing::ReaderOptions&)>> configList = {
            // 配置1：快速识别，只尝试常用格式
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::Code128 |
                                ZXing::BarcodeFormat::Code39);
                opts.setTryHarder(false);
                opts.setTryRotate(false);
                opts.setTryInvert(false);
            },

            // 配置2：标准识别，更多格式
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::Code128 |
                                ZXing::BarcodeFormat::Code39 | ZXing::BarcodeFormat::EAN13 |
                                ZXing::BarcodeFormat::DataMatrix | ZXing::BarcodeFormat::Aztec);
                opts.setTryHarder(true);
                opts.setTryRotate(false);
                opts.setTryInvert(false);
            },

            // 配置3：深度识别，启用旋转
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::Any);
                opts.setTryHarder(true);
                opts.setTryRotate(true);
                opts.setTryInvert(false);
            },

            // 配置4：最大努力识别，启用所有选项
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::Any);
                opts.setTryHarder(true);
                opts.setTryRotate(true);
                opts.setTryInvert(true);
                opts.setTryDownscale(true);
            }};

        // 按配置依次尝试识别
        for (size_t configIndex = 0; configIndex < configList.size(); ++configIndex)
        {
            ZXing::ReaderOptions options;
            configList[configIndex](options);

            qDebug() << "Trying recognition with config" << configIndex;

            auto result = ZXing::ReadBarcode(imageView, options);

            if (result.isValid())
            {
                QString recognizedText = QString::fromStdString(result.text());
                QString formatName = QString::fromStdString(ToString(result.format()));

                qDebug() << "Successfully recognized with config" << configIndex
                         << "Format:" << formatName << "Text length:" << recognizedText.length();

                // 返回格式信息和识别内容
                return recognizedText;
            }
        }

        // 如果所有配置都失败，尝试单独测试每种格式
        qDebug() << "Standard recognition failed, trying individual formats...";

        for (const auto& format : formatList)
        {
            ZXing::ReaderOptions options;
            options.setFormats(format);
            options.setTryHarder(true);
            options.setTryRotate(true);
            options.setTryInvert(true);

            auto result = ZXing::ReadBarcode(imageView, options);

            if (result.isValid())
            {
                QString recognizedText = QString::fromStdString(result.text());
                QString formatName = QString::fromStdString(ToString(result.format()));

                qDebug() << "Successfully recognized with individual format test"
                         << "Format:" << formatName << "Text length:" << recognizedText.length();

                return recognizedText;
            }
        }

        // 最后尝试：降低图像质量后重试
        qDebug() << "Trying with image preprocessing...";

        // 尝试灰度化和对比度增强
        QImage grayImage = rgbImage.convertToFormat(QImage::Format_Grayscale8);
        ZXing::ImageView grayImageView(grayImage.bits(), grayImage.width(), grayImage.height(),
                                       ZXing::ImageFormat::Lum, grayImage.bytesPerLine());

        ZXing::ReaderOptions finalOptions;
        finalOptions.setFormats(ZXing::BarcodeFormat::Any);
        finalOptions.setTryHarder(true);
        finalOptions.setTryRotate(true);
        finalOptions.setTryInvert(true);
        finalOptions.setTryDownscale(true);

        auto finalResult = ZXing::ReadBarcode(grayImageView, finalOptions);

        if (finalResult.isValid())
        {
            QString recognizedText = QString::fromStdString(finalResult.text());
            QString formatName = QString::fromStdString(ToString(finalResult.format()));

            qDebug() << "Successfully recognized with grayscale preprocessing"
                     << "Format:" << formatName << "Text length:" << recognizedText.length();

            return recognizedText;
        }

        return "未找到二维码或条码，请确保图片清晰且包含有效的码";
    }
    catch (const std::exception& e)
    {
        qDebug() << "Recognition error:" << e.what();
        return QString("识别错误：%1").arg(e.what());
    }
}

QRCodeDetectionResult MainWindow::detectQRCodeFromImage(const QImage& image)
{
    try
    {
        // 转换为ZXing可以处理的格式
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

        ZXing::ImageView imageView(rgbImage.bits(), rgbImage.width(), rgbImage.height(),
                                   ZXing::ImageFormat::RGB, rgbImage.bytesPerLine());

        // 定义常用的条码格式列表，按使用频率排序
        std::vector<ZXing::BarcodeFormat> formatList = {
            ZXing::BarcodeFormat::QRCode,      // 最常用的二维码
            ZXing::BarcodeFormat::Code128,     // 常用一维码
            ZXing::BarcodeFormat::Code39,      // 常用一维码
            ZXing::BarcodeFormat::EAN13,       // 商品条码
            ZXing::BarcodeFormat::DataMatrix,  // 数据矩阵
            ZXing::BarcodeFormat::Aztec,       // 阿兹特克码
            ZXing::BarcodeFormat::PDF417,      // PDF417
            ZXing::BarcodeFormat::Code93,      // Code93
            ZXing::BarcodeFormat::EAN8,        // EAN8
            ZXing::BarcodeFormat::UPCA,        // UPC-A
            ZXing::BarcodeFormat::UPCE,        // UPC-E
            ZXing::BarcodeFormat::ITF,         // ITF
            ZXing::BarcodeFormat::Codabar,     // Codabar
            ZXing::BarcodeFormat::MicroQRCode, // 微型QR码
            ZXing::BarcodeFormat::MaxiCode,    // MaxiCode
        };

        // 定义不同的识别配置，从宽松到严格
        std::vector<std::function<void(ZXing::ReaderOptions&)>> configList = {
            // 配置1：快速识别，只尝试常用格式
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::Code128 |
                                ZXing::BarcodeFormat::Code39);
                opts.setTryHarder(false);
                opts.setTryRotate(false);
                opts.setTryInvert(false);
            },

            // 配置2：标准识别，更多格式
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::Code128 |
                                ZXing::BarcodeFormat::Code39 | ZXing::BarcodeFormat::EAN13 |
                                ZXing::BarcodeFormat::DataMatrix | ZXing::BarcodeFormat::Aztec);
                opts.setTryHarder(true);
                opts.setTryRotate(false);
                opts.setTryInvert(false);
            },

            // 配置3：深度识别，启用旋转
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::Any);
                opts.setTryHarder(true);
                opts.setTryRotate(true);
                opts.setTryInvert(false);
            },

            // 配置4：最大努力识别，启用所有选项
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::Any);
                opts.setTryHarder(true);
                opts.setTryRotate(true);
                opts.setTryInvert(true);
                opts.setTryDownscale(true);
            }};

        // 按配置依次尝试识别
        for (size_t configIndex = 0; configIndex < configList.size(); ++configIndex)
        {
            ZXing::ReaderOptions options;
            configList[configIndex](options);

            qDebug() << "Trying recognition with config" << configIndex;

            auto result = ZXing::ReadBarcode(imageView, options);

            if (result.isValid())
            {
                QString recognizedText = QString::fromStdString(result.text());
                QString formatName = QString::fromStdString(ToString(result.format()));

                qDebug() << "Successfully recognized with config" << configIndex
                         << "Format:" << formatName << "Text length:" << recognizedText.length();

                // 返回包含位置信息的结果
                return QRCodeDetectionResult(recognizedText, result.position(), formatName);
            }
        }

        // 如果所有配置都失败，尝试单独测试每种格式
        qDebug() << "Standard recognition failed, trying individual formats...";

        for (const auto& format : formatList)
        {
            ZXing::ReaderOptions options;
            options.setFormats(format);
            options.setTryHarder(true);
            options.setTryRotate(true);
            options.setTryInvert(true);

            auto result = ZXing::ReadBarcode(imageView, options);

            if (result.isValid())
            {
                QString recognizedText = QString::fromStdString(result.text());
                QString formatName = QString::fromStdString(ToString(result.format()));

                qDebug() << "Successfully recognized with individual format test"
                         << "Format:" << formatName << "Text length:" << recognizedText.length();

                return QRCodeDetectionResult(recognizedText, result.position(), formatName);
            }
        }

        // 最后尝试：降低图像质量后重试
        qDebug() << "Trying with image preprocessing...";

        // 尝试灰度化和对比度增强
        QImage grayImage = rgbImage.convertToFormat(QImage::Format_Grayscale8);
        ZXing::ImageView grayImageView(grayImage.bits(), grayImage.width(), grayImage.height(),
                                       ZXing::ImageFormat::Lum, grayImage.bytesPerLine());

        ZXing::ReaderOptions finalOptions;
        finalOptions.setFormats(ZXing::BarcodeFormat::Any);
        finalOptions.setTryHarder(true);
        finalOptions.setTryRotate(true);
        finalOptions.setTryInvert(true);
        finalOptions.setTryDownscale(true);

        auto finalResult = ZXing::ReadBarcode(grayImageView, finalOptions);

        if (finalResult.isValid())
        {
            QString recognizedText = QString::fromStdString(finalResult.text());
            QString formatName = QString::fromStdString(ToString(finalResult.format()));

            qDebug() << "Successfully recognized with grayscale preprocessing"
                     << "Format:" << formatName << "Text length:" << recognizedText.length();

            return QRCodeDetectionResult(recognizedText, finalResult.position(), formatName);
        }

        return QRCodeDetectionResult(); // 返回无效结果
    }
    catch (const std::exception& e)
    {
        qDebug() << "Detection error:" << e.what();
        return QRCodeDetectionResult();
    }
}

QPixmap MainWindow::drawQRCodeContour(const QPixmap& originalPixmap, const ZXing::QuadrilateralI& position)
{
    if (originalPixmap.isNull())
        return originalPixmap;

    QPixmap result = originalPixmap.copy();
    QPainter painter(&result);
    
    // 设置绘制参数
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 设置轮廓样式
    QPen contourPen;
    contourPen.setColor(Qt::red);           // 红色轮廓
    contourPen.setWidth(3);                 // 线条宽度
    contourPen.setStyle(Qt::SolidLine);     // 实线
    painter.setPen(contourPen);
    
    // 获取四个角点（从整数转换为浮点数用于绘制）
    QPointF topLeft(static_cast<qreal>(position.topLeft().x), static_cast<qreal>(position.topLeft().y));
    QPointF topRight(static_cast<qreal>(position.topRight().x), static_cast<qreal>(position.topRight().y));
    QPointF bottomRight(static_cast<qreal>(position.bottomRight().x), static_cast<qreal>(position.bottomRight().y));
    QPointF bottomLeft(static_cast<qreal>(position.bottomLeft().x), static_cast<qreal>(position.bottomLeft().y));
    
    qDebug() << "Drawing contour at points:" 
             << topLeft << topRight << bottomRight << bottomLeft;
    
    // 绘制四边形轮廓
    QPolygonF polygon;
    polygon << topLeft << topRight << bottomRight << bottomLeft;
    painter.drawPolygon(polygon);
    
    // 在四个角点绘制小圆点以突出显示
    painter.setBrush(Qt::red);
    const int pointRadius = 5;
    painter.drawEllipse(topLeft, pointRadius, pointRadius);
    painter.drawEllipse(topRight, pointRadius, pointRadius);
    painter.drawEllipse(bottomRight, pointRadius, pointRadius);
    painter.drawEllipse(bottomLeft, pointRadius, pointRadius);
    
    // 添加文本标签显示格式信息（可选）
    QString formatText = QString("检测到: %1").arg(m_currentDetectionResult.format);
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    
    // 计算文本位置（在二维码上方）
    QPointF textPos = topLeft + QPointF(0, -10);
    if (textPos.y() < 20) {
        textPos.setY(bottomLeft.y() + 25); // 如果上方空间不够，放在下方
    }
    
    // 绘制文本背景
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(formatText);
    textRect.moveTo(textPos.toPoint());
    textRect.adjust(-5, -2, 5, 2);
    
    painter.setBrush(QColor(255, 255, 255, 200)); // 半透明白色背景
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 3, 3);
    
    // 绘制文本
    painter.setPen(Qt::blue);
    painter.drawText(textPos, formatText);
    
    painter.end();
    return result;
}

void MainWindow::displayRecognitionResult(const QString& result)
{
    m_resultTextEdit->clear();
    m_resultTextEdit->setPlainText(result);

    // 存储当前识别结果
    m_currentRecognizedText = result;

    if (result.startsWith("未找到") || result.startsWith("识别错误"))
    {
        m_resultTextEdit->setStyleSheet("QTextEdit { color: #dc3545; }");
        m_statusLabel->setText("识别失败");
    }
    else
    {
        m_resultTextEdit->setStyleSheet("QTextEdit { color: #28a745; }");
        m_statusLabel->setText("识别成功");
    }
}

void MainWindow::displaySelectedImage(const QPixmap& pixmap)
{
    // 缩放图片以适应显示区域
    QPixmap scaledPixmap =
        pixmap.scaled(m_imageDisplayLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageDisplayLabel->setPixmap(scaledPixmap);
}

void MainWindow::checkCameraPermissions()
{
    qDebug() << "Checking camera permissions...";

#ifdef Q_OS_WINDOWS
    qDebug() << "Windows platform - camera permissions usually handled by system";
#endif

#ifdef Q_OS_LINUX
    qDebug() << "Linux platform - check /dev/video* permissions";
#endif

#ifdef Q_OS_MACOS
    qDebug() << "macOS platform - check Privacy & Security settings";
#endif

    // 检查Qt多媒体模块是否正确加载
    qDebug() << "Available video inputs count:" << QMediaDevices::videoInputs().size();

    // 输出所有可用的媒体设备信息
    debugCameraInfo();
}

void MainWindow::debugCameraInfo()
{
    qDebug() << "=== Camera Debug Information ===";

    auto videoInputs = QMediaDevices::videoInputs();
    qDebug() << "Total video input devices:" << videoInputs.size();

    for (int i = 0; i < videoInputs.size(); ++i)
    {
        const QCameraDevice& device = videoInputs.at(i);
        qDebug() << "Device" << i << ":";
        qDebug() << "  Description:" << device.description();
        qDebug() << "  ID:" << device.id();
        qDebug() << "  IsDefault:" << device.isDefault();
        qDebug() << "  Position:" << static_cast<int>(device.position());

        // 输出支持的视频格式
        auto formats = device.videoFormats();
        qDebug() << "  Supported formats:" << formats.size();
        for (const auto& format : formats)
        {
            qDebug() << "    Resolution:" << format.resolution() << "FPS:" << format.minFrameRate()
                     << "-" << format.maxFrameRate();
        }
    }

    qDebug() << "=== End Camera Debug Information ===";
}

void MainWindow::onToggleCamera()
{
    if (m_cameraActive)
    {
        stopCamera();
    }
    else
    {
        startCamera();
    }
}

QPixmap MainWindow::generateQRCodeWithFallback(const QString& text, const QString& ecLevel,
                                               int size)
{
    qDebug() << "Using fallback QR code generation method";

    try
    {
        ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);
        writer.setMargin(2);

        // 简化的纠错级别设置
        int ecLevelInt = 1; // 默认M级别
        if (ecLevel.toUpper() == "L")
            ecLevelInt = 0;
        else if (ecLevel.toUpper() == "M")
            ecLevelInt = 1;
        else if (ecLevel.toUpper() == "Q")
            ecLevelInt = 2;
        else if (ecLevel.toUpper() == "H")
            ecLevelInt = 3;

        writer.setEccLevel(ecLevelInt);
        writer.setEncoding(ZXing::CharacterSet::UTF8);

        QByteArray utf8Data = text.toUtf8();
        std::string stdText = utf8Data.toStdString();

        // 确保尺寸合理
        int actualSize = qMax(100, qMin(800, size));

        auto bitMatrix = writer.encode(stdText, actualSize, actualSize);

        if (bitMatrix.width() > 0 && bitMatrix.height() > 0)
        {
            return zxingMatrixToQPixmap(bitMatrix);
        }
        else
        {
            qDebug() << "Generated BitMatrix has invalid dimensions";
            return QPixmap();
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "Fallback generation failed:" << e.what();
        return QPixmap();
    }
}

QPixmap MainWindow::generateMinimalQRCode(const QString& text)
{
    qDebug() << "Using minimal QR code generation";

    try
    {
        // 创建最基本的writer，不设置任何可选参数
        ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);

        // 转换文本为标准字符串
        std::string stdText = text.toStdString();

        qDebug() << "Minimal: generating with basic parameters only";

        // 使用默认参数生成（不调用任何setter方法）
        auto bitMatrix = writer.encode(stdText, 200, 200);

        if (bitMatrix.width() > 0 && bitMatrix.height() > 0)
        {
            qDebug() << "Minimal generation successful, size:" << bitMatrix.width() << "x"
                     << bitMatrix.height();
            return zxingMatrixToQPixmap(bitMatrix);
        }
        else
        {
            qDebug() << "Minimal generation failed: invalid matrix dimensions";
            return createErrorQRCode();
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "Minimal generation exception:" << e.what();
        return createErrorQRCode();
    }
}

QPixmap MainWindow::createErrorQRCode()
{
    qDebug() << "Creating error placeholder QR code";

    // 创建一个简单的错误提示图像
    int size = 200;
    QPixmap errorPixmap(size, size);
    errorPixmap.fill(Qt::white);

    QPainter painter(&errorPixmap);
    painter.setPen(QPen(Qt::red, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));

    // 绘制错误信息
    painter.drawText(errorPixmap.rect(), Qt::AlignCenter, "二维码\n生成失败");

    // 绘制边框
    painter.drawRect(0, 0, size - 1, size - 1);

    return errorPixmap;
}

void MainWindow::addToHistory(const QString& result)
{
    // 检查是否与上次识别结果相同
    if (result == m_lastRecognizedContent)
    {
        return; // 相同结果不添加到历史记录
    }

    // 更新最后识别的内容
    m_lastRecognizedContent = result;

    // 添加到历史记录（带时间戳）
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString historyEntry = QString("[%1] %2").arg(timestamp).arg(result);

    m_recognitionHistory.append(historyEntry);

    // 限制历史记录数量（保留最近50条）
    if (m_recognitionHistory.size() > 50)
    {
        m_recognitionHistory.removeFirst();
    }

    // 更新显示
    updateHistoryDisplay();
}

void MainWindow::updateHistoryDisplay()
{
    if (m_recognitionHistory.isEmpty())
    {
        m_historyTextEdit->setPlainText("暂无识别记录");
        return;
    }

    // 倒序显示（最新的在上面）
    QStringList reversedHistory = m_recognitionHistory;
    std::reverse(reversedHistory.begin(), reversedHistory.end());

    QString displayText = reversedHistory.join("\n\n");
    m_historyTextEdit->setPlainText(displayText);

    // 滚动到顶部显示最新记录
    QTextCursor cursor = m_historyTextEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_historyTextEdit->setTextCursor(cursor);
}

void MainWindow::showRecognitionHint(const QString& result)
{
    // 停止之前的定时器
    m_hintTimer->stop();

    // 设置提示文本
    QString hintText;
    if (result == m_lastRecognizedContent && m_recognitionHistory.size() > 1)
    {
        hintText = "✓ 检测到相同二维码，未重复记录";
    }
    else
    {
        hintText = QString("✓ 新识别到二维码：%1")
                       .arg(result.length() > 30 ? result.left(30) + "..." : result);

        // 添加自动操作提示
        QStringList actions;
        if (m_autoCopyEnabled)
        {
            actions << "已复制";
        }
        if (m_autoOpenUrlEnabled && isValidUrl(result))
        {
            actions << "已打开网址";
        }

        if (!actions.isEmpty())
        {
            hintText += QString(" (%1)").arg(actions.join(", "));
        }
    }

    m_recognitionHintLabel->setText(hintText);
    m_recognitionHintLabel->show();

    // 3秒后自动隐藏
    m_hintTimer->start();
}

void MainWindow::onAutoOpenUrlChanged(bool enabled)
{
    m_autoOpenUrlEnabled = enabled;
    qDebug() << "Auto open URL changed to:" << enabled;
}

void MainWindow::onClearHistory()
{
    int ret = QMessageBox::question(this, "确认清空", "确定要清空所有识别历史记录吗？",
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        m_recognitionHistory.clear();
        m_lastRecognizedContent.clear();
        updateHistoryDisplay();

        // 显示清空成功提示
        m_recognitionHintLabel->setText("✓ 历史记录已清空");
        m_recognitionHintLabel->show();
        m_hintTimer->start();
    }
}
void MainWindow::onAutoCopyChanged(bool enabled)
{
    m_autoCopyEnabled = enabled;
    qDebug() << "Auto copy changed to:" << enabled;
}

void MainWindow::handleAutoActions(const QString& result)
{
    // 自动复制到剪贴板
    if (m_autoCopyEnabled)
    {
        autoCopyToClipboard(result);
    }

    // 自动打开网址
    if (m_autoOpenUrlEnabled && isValidUrl(result))
    {
        autoOpenUrl(result);
    }
}

bool MainWindow::isValidUrl(const QString& text)
{
    QUrl url(text);

    // 检查是否为有效的URL且协议为http或https
    if (!url.isValid())
    {
        return false;
    }

    QString scheme = url.scheme().toLower();
    if (scheme != "http" && scheme != "https" && scheme != "ftp")
    {
        // 尝试添加http://前缀
        url.setUrl("http://" + text);
        if (!url.isValid())
        {
            return false;
        }
        scheme = url.scheme().toLower();
    }

    // 检查是否有有效的主机名
    QString host = url.host();
    if (host.isEmpty())
    {
        return false;
    }

    // 简单的域名格式检查
    if (host.contains('.') && host.length() > 3)
    {
        return true;
    }

    return false;
}

void MainWindow::autoOpenUrl(const QString& url)
{
    try
    {
        QString processedUrl = url;

        // 如果URL没有协议前缀，添加http://
        if (!processedUrl.startsWith("http://") && !processedUrl.startsWith("https://") &&
            !processedUrl.startsWith("ftp://"))
        {
            processedUrl = "http://" + processedUrl;
        }

        QUrl qurl(processedUrl);
        if (qurl.isValid())
        {
            bool success = QDesktopServices::openUrl(qurl);
            if (success)
            {
                qDebug() << "Successfully opened URL:" << processedUrl;
            }
            else
            {
                qDebug() << "Failed to open URL:" << processedUrl;
                // 更新提示显示失败信息
                m_recognitionHintLabel->setText("✗ 打开网址失败");
                m_recognitionHintLabel->setStyleSheet(
                    "QLabel { background-color: #f8d7da; color: #721c24; border: 1px solid "
                    "#f5c6cb; "
                    "border-radius: 5px; padding: 8px; margin: 5px; }");
                m_recognitionHintLabel->show();
                m_hintTimer->start();
            }
        }
        else
        {
            qDebug() << "Invalid URL format:" << processedUrl;
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "Exception opening URL:" << e.what();
    }
}

void MainWindow::autoCopyToClipboard(const QString& text)
{
    try
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            clipboard->setText(text);
            qDebug() << "Content copied to clipboard:" << text.left(50) + "...";
        }
        else
        {
            qDebug() << "Failed to access clipboard";
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "Exception copying to clipboard:" << e.what();
    }
}
void MainWindow::updateRecognizeActionButtons(const QString& result)
{
    // 检查是否识别成功
    bool recognitionSuccess = !result.startsWith("未找到") && !result.startsWith("识别错误");

    // 复制按钮：识别成功时启用
    m_copyResultButton->setEnabled(recognitionSuccess);

    // 打开网址按钮：识别成功且内容是有效网址时启用
    bool isUrl = recognitionSuccess && isValidUrl(result);
    m_openUrlButton->setEnabled(isUrl);

    // 更新按钮文本和提示
    if (recognitionSuccess)
    {
        m_copyResultButton->setText("复制内容");
        m_copyResultButton->setToolTip(
            QString("复制识别结果：%1")
                .arg(result.length() > 50 ? result.left(50) + "..." : result));

        if (isUrl)
        {
            m_openUrlButton->setText("打开网址");
            m_openUrlButton->setToolTip(QString("在浏览器中打开：%1").arg(result));
        }
        else
        {
            m_openUrlButton->setText("非网址内容");
            m_openUrlButton->setToolTip("识别结果不是有效的网址格式");
        }
    }
    else
    {
        m_copyResultButton->setText("复制内容");
        m_copyResultButton->setToolTip("识别成功后可复制内容");

        m_openUrlButton->setText("打开网址");
        m_openUrlButton->setToolTip("识别成功后如果是网址可以打开");
    }
}

void MainWindow::onCopyRecognizedText()
{
    if (m_currentRecognizedText.isEmpty() || m_currentRecognizedText.startsWith("未找到") ||
        m_currentRecognizedText.startsWith("识别错误"))
    {
        QMessageBox::warning(this, "提示", "没有可复制的有效内容！");
        return;
    }

    try
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            clipboard->setText(m_currentRecognizedText);

            // 显示成功提示
            m_statusLabel->setText("内容已复制到剪贴板");
            QMessageBox::information(this, "复制成功",
                                     QString("已将以下内容复制到剪贴板：\n\n%1")
                                         .arg(m_currentRecognizedText.length() > 200
                                                  ? m_currentRecognizedText.left(200) + "..."
                                                  : m_currentRecognizedText));

            qDebug() << "Content copied to clipboard from recognize mode:"
                     << m_currentRecognizedText;
        }
        else
        {
            QMessageBox::warning(this, "复制失败", "无法访问系统剪贴板！");
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "复制失败", QString("复制时发生错误：%1").arg(e.what()));
        qDebug() << "Exception copying to clipboard in recognize mode:" << e.what();
    }
}

void MainWindow::onOpenRecognizedUrl()
{
    if (m_currentRecognizedText.isEmpty() || m_currentRecognizedText.startsWith("未找到") ||
        m_currentRecognizedText.startsWith("识别错误"))
    {
        QMessageBox::warning(this, "提示", "没有可打开的有效网址！");
        return;
    }

    if (!isValidUrl(m_currentRecognizedText))
    {
        QMessageBox::warning(this, "提示", "识别结果不是有效的网址格式！");
        return;
    }

    try
    {
        QString processedUrl = m_currentRecognizedText;

        // 如果URL没有协议前缀，添加http://
        if (!processedUrl.startsWith("http://") && !processedUrl.startsWith("https://") &&
            !processedUrl.startsWith("ftp://"))
        {
            processedUrl = "http://" + processedUrl;
        }

        QUrl qurl(processedUrl);
        if (qurl.isValid())
        {
            // 确认对话框
            int ret = QMessageBox::question(
                this, "确认打开",
                QString("确定要在浏览器中打开以下网址吗？\n\n%1").arg(processedUrl),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (ret == QMessageBox::Yes)
            {
                bool success = QDesktopServices::openUrl(qurl);
                if (success)
                {
                    m_statusLabel->setText("网址已在浏览器中打开");
                    qDebug() << "Successfully opened URL from recognize mode:" << processedUrl;
                }
                else
                {
                    QMessageBox::warning(this, "打开失败",
                                         "无法打开网址，请检查系统设置或手动复制网址到浏览器。");
                    qDebug() << "Failed to open URL from recognize mode:" << processedUrl;
                }
            }
        }
        else
        {
            QMessageBox::warning(this, "网址错误", "网址格式无效，无法打开。");
            qDebug() << "Invalid URL format in recognize mode:" << processedUrl;
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "打开失败", QString("打开网址时发生错误：%1").arg(e.what()));
        qDebug() << "Exception opening URL in recognize mode:" << e.what();
    }
}
void MainWindow::onImageCaptured(int id, const QImage& image)
{
    Q_UNUSED(id)

    if (image.isNull())
    {
        return;
    }

    if (index_ != 2)
    {
        // 手动拍照：同步处理并显示结果
        QString result = recognizeQRCodeFromImage(image);

        // 更新实时结果显示
        m_cameraResultTextEdit->clear();
        if (result.startsWith("未找到") || result.startsWith("识别错误"))
        {
            m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #dc3545; }");
            m_cameraResultTextEdit->setPlainText(QString("拍照识别结果：%1").arg(result));
        }
        else
        {
            m_cameraResultTextEdit->setStyleSheet(
                "QTextEdit { color: #28a745; font-weight: bold; }");
            m_cameraResultTextEdit->setPlainText(QString("拍照识别成功！\n内容：%1").arg(result));

            // 新增：添加到历史记录
            addToHistory(result);

            // 新增：处理自动操作
            handleAutoActions(result);

            // 新增：显示识别提示
            showRecognitionHint(result);
        }

        m_lastCaptureRequestId = id;
    }
    else
    {
        // 实时识别：异步处理
        startAsyncRecognition(image);
    }
}

// void MainWindow::onRecognitionTimerTimeout()
// {
//     // 实时识别（每秒触发一次）
//     if (m_cameraActive && m_imageCapture)
//     {
//         // 避免过于频繁的捕获，检查上次识别时间
//         qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
//         if (currentTime - m_lastRecognitionTime < 1000)
//         { // 1秒内不重复识别
//             return;
//         }

//         m_lastRecognitionTime = currentTime;
//         m_imageCapture->capture();
//     }
// }

void MainWindow::startAsyncRecognition(const QImage& image)
{
    if (!m_imageProcessor)
    {
        return;
    }

    // 将图像添加到异步处理队列
    m_imageProcessor->addImage(image);
}

void MainWindow::onAsyncRecognitionResult(const QString& result, int requestId)
{
    Q_UNUSED(requestId)

    // 更新识别计数
    m_recognitionCount++;

    // 只在识别成功时更新UI和执行操作
    if (!result.startsWith("未找到") && !result.startsWith("识别错误"))
    {
        QMutexLocker locker(&m_recognitionMutex);

        // 检查是否是新的识别结果
        if (result != m_lastRecognizedContent)
        {
            // 更新实时结果显示
            m_cameraResultTextEdit->clear();
            m_cameraResultTextEdit->setStyleSheet(
                "QTextEdit { color: #28a745; font-weight: bold; }");
            m_cameraResultTextEdit->setPlainText(QString("实时识别成功！\n内容：%1").arg(result));

            // 添加到历史记录
            addToHistory(result);

            // 处理自动操作
            handleAutoActions(result);

            // 显示识别提示
            showRecognitionHint(result);

            qDebug() << "Async recognition successful:" << result.left(50)
                     << "requestId:" << requestId << "count:" << m_recognitionCount;
        }
    }

    // 每100次识别输出一次性能统计
    if (m_recognitionCount % 100 == 0)
    {
        qDebug() << "Recognition performance: processed" << m_recognitionCount << "images";
    }
}

// 新增：QRRecognitionWorker 实现
QRRecognitionWorker::QRRecognitionWorker(QObject* parent) : QObject(parent), m_shouldStop(false)
{
    qDebug() << "QRRecognitionWorker created in thread:" << QThread::currentThread();
}

QRRecognitionWorker::~QRRecognitionWorker()
{
    qDebug() << "QRRecognitionWorker destroyed";
}

void QRRecognitionWorker::processImage(const QImage& image, int requestId)
{
    QMutexLocker locker(&m_mutex);

    if (m_shouldStop)
    {
        return;
    }

    qDebug() << "Worker processing image, requestId:" << requestId
             << "thread:" << QThread::currentThread();

    try
    {
        QString result = recognizeQRCodeFromImage(image);

        if (!m_shouldStop)
        {
            emit recognitionCompleted(result, requestId);
        }
    }
    catch (const std::exception& e)
    {
        if (!m_shouldStop)
        {
            emit recognitionFailed(QString("Recognition error: %1").arg(e.what()), requestId);
        }
    }
}

void QRRecognitionWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_shouldStop = true;
    qDebug() << "QRRecognitionWorker stop requested";
}

QString QRRecognitionWorker::recognizeQRCodeFromImage(const QImage& image)
{
    // 这里复制原来MainWindow中的识别逻辑
    try
    {
        // 转换为ZXing可以处理的格式
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

        ZXing::ImageView imageView(rgbImage.bits(), rgbImage.width(), rgbImage.height(),
                                   ZXing::ImageFormat::RGB, rgbImage.bytesPerLine());

        // 定义常用的条码格式列表，按使用频率排序
        std::vector<ZXing::BarcodeFormat> formatList = {
            ZXing::BarcodeFormat::QRCode,     // 最常用的二维码
            ZXing::BarcodeFormat::Code128,    // 常用一维码
            ZXing::BarcodeFormat::Code39,     // 常用一维码
            ZXing::BarcodeFormat::EAN13,      // 商品条码
            ZXing::BarcodeFormat::DataMatrix, // 数据矩阵
        };

        // 针对实时识别优化的配置
        std::vector<std::function<void(ZXing::ReaderOptions&)>> configList = {
            // 配置1：快速识别，只尝试QR码
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::QRCode);
                opts.setTryHarder(false);
                opts.setTryRotate(false);
                opts.setTryInvert(false);
            },

            // 配置2：中等速度，常用格式
            [](ZXing::ReaderOptions& opts)
            {
                opts.setFormats(ZXing::BarcodeFormat::QRCode | ZXing::BarcodeFormat::Code128 |
                                ZXing::BarcodeFormat::Code39 | ZXing::BarcodeFormat::EAN13);
                opts.setTryHarder(true);
                opts.setTryRotate(false);
                opts.setTryInvert(false);
            },
        };

        // 按配置依次尝试识别
        for (size_t configIndex = 0; configIndex < configList.size(); ++configIndex)
        {
            ZXing::ReaderOptions options;
            configList[configIndex](options);

            auto result = ZXing::ReadBarcode(imageView, options);

            if (result.isValid())
            {
                QString recognizedText = QString::fromStdString(result.text());
                return recognizedText;
            }
        }

        return "未找到二维码或条码";
    }
    catch (const std::exception& e)
    {
        return QString("识别错误：%1").arg(e.what());
    }
}

// 新增：ImageProcessor 实现
ImageProcessor::ImageProcessor(QObject* parent)
    : QObject(parent), m_processTimer(new QTimer(this)), m_workerThread(nullptr), m_worker(nullptr),
      m_currentRequestId(0), m_lastProcessedRequestId(-1)
{
    m_processTimer->setSingleShot(false);
    m_processTimer->setInterval(PROCESSING_INTERVAL);
    connect(m_processTimer, &QTimer::timeout, this, &ImageProcessor::processNextImage);
    qDebug() << "ImageProcessor created in thread:" << QThread::currentThread();
}

ImageProcessor::~ImageProcessor()
{
    stop();
}

void ImageProcessor::addImage(const QImage& image)
{
    QMutexLocker locker(&m_queueMutex);

    // 限制队列大小，丢弃旧图像
    while (m_imageQueue.size() >= MAX_QUEUE_SIZE)
    {
        m_imageQueue.dequeue();
        qDebug() << "Dropped old image from queue";
    }

    ImageRequest request;
    request.image = image.copy(); // 深拷贝确保线程安全
    request.requestId = ++m_currentRequestId;
    request.timestamp = QDateTime::currentMSecsSinceEpoch();

    m_imageQueue.enqueue(request);

    qDebug() << "Added image to queue, requestId:" << request.requestId
             << "queue size:" << m_imageQueue.size();
}

void ImageProcessor::start()
{
    if (m_workerThread)
    {
        qDebug() << "ImageProcessor already started";
        return;
    }

    // 创建工作线程
    m_workerThread = new QThread();
    m_worker = new QRRecognitionWorker();
    m_worker->moveToThread(m_workerThread);

    // 连接信号槽
    connect(this, &ImageProcessor::imageReady, m_worker, &QRRecognitionWorker::processImage);
    connect(m_worker, &QRRecognitionWorker::recognitionCompleted, this,
            &ImageProcessor::onRecognitionCompleted);
    connect(m_worker, &QRRecognitionWorker::recognitionFailed, this,
            &ImageProcessor::onRecognitionFailed);

    // 启动线程和定时器
    m_workerThread->start();
    m_processTimer->start();

    qDebug() << "ImageProcessor started, worker thread:" << m_workerThread;
}

void ImageProcessor::stop()
{
    if (m_processTimer)
    {
        m_processTimer->stop();
    }

    if (m_worker)
    {
        m_worker->stop();
    }

    if (m_workerThread)
    {
        m_workerThread->quit();
        if (!m_workerThread->wait(3000))
        {
            qWarning() << "Worker thread did not finish within timeout, terminating";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }

        delete m_worker;
        m_worker = nullptr;

        delete m_workerThread;
        m_workerThread = nullptr;
    }

    QMutexLocker locker(&m_queueMutex);
    m_imageQueue.clear();

    qDebug() << "ImageProcessor stopped";
}

void ImageProcessor::setProcessingInterval(int ms)
{
    m_processTimer->setInterval(ms);
    qDebug() << "Processing interval set to:" << ms << "ms";
}

void ImageProcessor::processNextImage()
{
    QMutexLocker locker(&m_queueMutex);

    if (m_imageQueue.isEmpty() || !m_worker)
    {
        return;
    }

    ImageRequest request = m_imageQueue.dequeue();
    locker.unlock();

    // 检查图像是否太旧（超过5秒丢弃）
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - request.timestamp > 5000)
    {
        qDebug() << "Dropped old image, requestId:" << request.requestId;
        return;
    }

    qDebug() << "Processing image, requestId:" << request.requestId;
    emit imageReady(request.image, request.requestId);
}

void ImageProcessor::onRecognitionCompleted(const QString& result, int requestId)
{
    // 确保只处理最新的识别结果
    if (requestId > m_lastProcessedRequestId)
    {
        m_lastProcessedRequestId = requestId;
        emit recognitionResult(result, requestId);
        qDebug() << "Recognition completed, requestId:" << requestId
                 << "result:" << result.left(50);
    }
    else
    {
        qDebug() << "Ignored old recognition result, requestId:" << requestId;
    }
}

void ImageProcessor::onRecognitionFailed(const QString& error, int requestId)
{
    qDebug() << "Recognition failed, requestId:" << requestId << "error:" << error;
    // 失败时也可以发送信号，但通常我们忽略失败的结果
}

// void MainWindow::startCamera()
// {
//     qDebug() << "Starting camera...";

//     if (m_availableCameras.isEmpty())
//     {
//         QMessageBox::warning(this, "错误", "未找到可用的摄像头！\n请检查摄像头连接和权限设置。");
//         return;
//     }

//     try
//     {
//         // 停止之前的摄像头
//         if (m_camera)
//         {
//             m_camera->stop();
//             m_camera->deleteLater();
//             m_camera = nullptr;
//         }

//         int selectedIndex = m_cameraComboBox->currentIndex();
//         if (selectedIndex < 0 || selectedIndex >= m_availableCameras.size())
//         {
//             QMessageBox::warning(this, "错误", "无效的摄像头选择！");
//             return;
//         }

//         const QCameraDevice& selectedCamera = m_availableCameras.at(selectedIndex);
//         qDebug() << "Selected camera:" << selectedCamera.description();

//         // 创建摄像头
//         m_camera = new QCamera(selectedCamera, this);

//         // 连接错误信号
//         connect(m_camera, &QCamera::errorOccurred, this, &MainWindow::onCameraError);

//         // 连接状态变化信号
//         connect(m_camera, &QCamera::activeChanged, this,
//                 [this](bool active)
//                 {
//                     qDebug() << "Camera active state changed:" << active;
//                     if (active)
//                     {
//                         m_cameraStatusLabel->setText("摄像头已启动 - 实时识别中...");

//                         // 启动异步图像处理器
//                         m_imageProcessor->start();
//                     }
//                 });

//         // 设置摄像头到捕获会话
//         m_captureSession->setCamera(m_camera);

//         // 检查摄像头是否可用
//         if (!m_camera->isAvailable())
//         {
//             QMessageBox::warning(this, "错误",
//                                  "选择的摄像头当前不可用！\n可能正被其他应用程序使用。");
//             return;
//         }

//         // 启动摄像头
//         m_camera->start();

//         // 等待摄像头启动
//         QTimer::singleShot(
//             1000, this,
//             [this]()
//             {
//                 if (m_camera && m_camera->isActive())
//                 {
//                     m_cameraActive = true;
//                     m_toggleCameraButton->setText("停止摄像头");
//                     m_toggleCameraButton->setStyleSheet(
//                         "QPushButton { font-size: 14px; padding: 10px 20px; background-color: "
//                         "#dc3545; color: white; border: none; border-radius: 5px; } "
//                         "QPushButton:hover { background-color: #c82333; }");
//                     m_captureButton->setEnabled(true);
//                     m_cameraComboBox->setEnabled(false);

//                     // 启动实时识别定时器 - 降低频率减少CPU占用
//                     m_recognitionTimer->start();

//                     // 设置处理间隔为1.5秒，避免过于频繁的识别
//                     m_imageProcessor->setProcessingInterval(1500);

//                     qDebug() << "Camera started successfully";
//                 }
//                 else
//                 {
//                     QMessageBox::warning(this, "错误",
//                                          "摄像头启动失败！\n请检查摄像头权限和驱动程序。");
//                 }
//             });
//     }
//     catch (const std::exception& e)
//     {
//         QMessageBox::critical(this, "错误", QString("启动摄像头失败：%1").arg(e.what()));
//         qDebug() << "Camera start exception:" << e.what();
//     }
// }

void MainWindow::stopCamera()
{
    // 停止异步处理器
    if (m_imageProcessor)
    {
        m_imageProcessor->stop();
    }

    if (m_camera)
    {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }

    m_recognitionTimer->stop();

    m_cameraActive = false;
    m_toggleCameraButton->setText("启动摄像头");
    m_toggleCameraButton->setStyleSheet(
        "QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: "
        "white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; "
        "}");
    m_captureButton->setEnabled(false);
    m_cameraComboBox->setEnabled(true);
    m_cameraStatusLabel->setText("摄像头已停止");
}

void MainWindow::onRecognitionTimerTimeout()
{
    // 实时识别（每秒触发一次）
    if (m_cameraActive && m_imageCapture)
    {
        // 避免过于频繁的捕获，检查上次识别时间
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - m_lastRecognitionTime < 1000)
        { // 1秒内不重复识别
            return;
        }

        m_lastRecognitionTime = currentTime;
        m_imageCapture->capture();
    }
}

// void MainWindow::startAsyncRecognition(const QImage& image)
// {
//     if (!m_imageProcessor)
//     {
//         return;
//     }

//     // 将图像添加到异步处理队列
//     m_imageProcessor->addImage(image);
// }

// void MainWindow::onAsyncRecognitionResult(const QString& result, int requestId)
// {
//     Q_UNUSED(requestId)

//     // 更新识别计数
//     m_recognitionCount++;

//     // 只在识别成功时更新UI和执行操作
//     if (!result.startsWith("未找到") && !result.startsWith("识别错误"))
//     {
//         QMutexLocker locker(&m_recognitionMutex);

//         // 检查是否是新的识别结果
//         if (result != m_lastRecognizedContent)
//         {
//             // 更新实时结果显示
//             m_cameraResultTextEdit->clear();
//             m_cameraResultTextEdit->setStyleSheet(
//                 "QTextEdit { color: #28a745; font-weight: bold; }");
//             m_cameraResultTextEdit->setPlainText(QString("实时识别成功！\n内容：%1").arg(result));

//             // 添加到历史记录
//             addToHistory(result);

//             // 处理自动操作
//             handleAutoActions(result);

//             // 显示识别提示
//             showRecognitionHint(result);

//             qDebug() << "Async recognition successful:" << result.left(50)
//                      << "requestId:" << requestId << "count:" << m_recognitionCount;
//         }
//     }

//     // 每100次识别输出一次性能统计
//     if (m_recognitionCount % 100 == 0)
//     {
//         qDebug() << "Recognition performance: processed" << m_recognitionCount << "images";
//     }
// }