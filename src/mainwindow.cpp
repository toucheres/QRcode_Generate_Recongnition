#include "mainwindow.h"
#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#include <QApplication>
#include <QCameraDevice>
#include <QDebug>
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
#include <iostream>
#include<QSvgGenerator>
//保存槽函数
void MainWindow::onSaveQRCode()
{
    // 检查 QLabel 是否已初始化
    if (!m_qrCodeLabel) {
        QMessageBox::warning(this, "错误", "QLabel 未初始化！");
        return;     
    }

    // 获取 QLabel 中的图片
    const QPixmap* currentPixmap = new QPixmap{ m_qrCodeLabel->pixmap()};
    if (!currentPixmap || currentPixmap->isNull()) {
        QMessageBox::warning(this, "保存失败", "请先生成二维码！");
        return;
    }

    // 创建副本
    QPixmap pixmapToSave = *currentPixmap;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("保存二维码图片"),
        "",
        tr("PNG 图片 (*.png);;JPEG 图片 (*.jpg);;SVG 矢量图 (*.svg)")
    );
    
    if (fileName.isEmpty())
        return;

    bool success = false;
    if (fileName.endsWith(".svg", Qt::CaseInsensitive)) {
        QSvgGenerator svgGen;
        svgGen.setFileName(fileName);
        svgGen.setSize(pixmapToSave.size());
        svgGen.setViewBox(QRect(0, 0, pixmapToSave.width(), pixmapToSave.height()));
        QPainter painter(&svgGen);
        painter.drawPixmap(0, 0, pixmapToSave);
        painter.end();
        success = true;
    } else {
        success = pixmapToSave.save(fileName);
    }

    if (success)
        QMessageBox::information(this, "保存成功", "二维码图片已保存！");
    else
        QMessageBox::warning(this, "保存失败", "保存二维码图片失败！");
}
//保存按钮实现
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
    m_qrCodeLabel->setPixmap(qrPixmap.scaled(m_qrCodeLabel->size(), 
        Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
      m_qrformat(nullptr)
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
    m_generateButton = new QPushButton("生成二维码", m_generateWidget);
    m_saveQRCodeButton = new QPushButton("保存二维码", m_generateWidget);
    m_saveQRCodeButton->setEnabled(false);
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
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateQRCode);
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

    // 结果显示
    QGroupBox* resultGroup = new QGroupBox("识别结果");
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
    m_resultTextEdit = new QTextEdit();
    m_resultTextEdit->setPlaceholderText("识别结果将显示在这里...");
    m_resultTextEdit->setStyleSheet("QTextEdit { font-size: 14px; padding: 10px; }");
    resultLayout->addWidget(m_resultTextEdit);

    splitter->addWidget(imageGroup);
    splitter->addWidget(resultGroup);
    splitter->setSizes({350, 350});

    // 状态标签
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");

    // 连接信号
    connect(m_selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectImageFile);
    connect(m_urlInput, &QLineEdit::textChanged, this, &MainWindow::onUrlInputChanged);
    connect(m_loadUrlButton, &QPushButton::clicked, this, &MainWindow::onLoadFromUrl);
    connect(m_urlInput, &QLineEdit::returnPressed, this, &MainWindow::onLoadFromUrl);

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

    // 结果显示区域
    QGroupBox* resultGroup = new QGroupBox("识别结果");
    QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
    m_cameraResultTextEdit = new QTextEdit();
    m_cameraResultTextEdit->setPlaceholderText(
        "实时识别结果将显示在这里...\n启动摄像头后会自动尝试识别画面中的二维码");
    m_cameraResultTextEdit->setStyleSheet("QTextEdit { font-size: 14px; padding: 10px; }");
    resultLayout->addWidget(m_cameraResultTextEdit);

    splitter->addWidget(videoGroup);
    splitter->addWidget(resultGroup);
    splitter->setSizes({450, 350});

    // 状态标签
    m_cameraStatusLabel = new QLabel("摄像头未启动");
    m_cameraStatusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");

    // 连接信号
    connect(m_toggleCameraButton, &QPushButton::clicked, this, &MainWindow::onToggleCamera);
    connect(m_captureButton, &QPushButton::clicked, this, &MainWindow::onCaptureImage);
    connect(m_cameraComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::onCameraChanged);

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

                    // 启动实时识别定时器
                    m_recognitionTimer->start();

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

void MainWindow::stopCamera()
{
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

void MainWindow::onImageCaptured(int id, const QImage& image)
{
    Q_UNUSED(id)

    if (image.isNull())
    {
        return;
    }

    QString result = recognizeQRCodeFromImage(image);

    // 更新结果显示
    m_cameraResultTextEdit->clear();
    if (result.startsWith("未找到") || result.startsWith("识别错误"))
    {
        m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #dc3545; }");
        m_cameraResultTextEdit->setPlainText(QString("拍照识别结果：%1").arg(result));
    }
    else
    {
        m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #28a745; font-weight: bold; }");
        m_cameraResultTextEdit->setPlainText(QString("拍照识别成功！\n内容：%1").arg(result));

        // 播放成功提示音（可选）
        QMessageBox::information(this, "识别成功", QString("识别到二维码内容：\n%1").arg(result));
    }
}

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

void MainWindow::onRecognitionTimerTimeout()
{
    // 实时识别（每秒触发一次）
    if (m_cameraActive && m_imageCapture)
    {
        recognizeFromVideoFrame();
    }
}

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
                        {"MicroQRCode", ZXing::BarcodeFormat::MicroQRCode},
                        {"RMQRCode", ZXing::BarcodeFormat::RMQRCode},
                        {"DataBar", ZXing::BarcodeFormat::DataBar},
                        {"DataBarExpanded", ZXing::BarcodeFormat::DataBarExpanded},
                        {"DataBarLimited", ZXing::BarcodeFormat::DataBarLimited},
                        {"DXFilmEdge", ZXing::BarcodeFormat::DXFilmEdge}};
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

        displaySelectedImage(pixmap);
        QString result = recognizeQRCodeFromPixmap(pixmap);
        displayRecognitionResult(result);
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
    displaySelectedImage(pixmap);
    QString result = recognizeQRCodeFromPixmap(pixmap);
    displayRecognitionResult(result);

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void MainWindow::onModeChanged(int index)
{
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

        ZXing::ReaderOptions options;
        options.setFormats(ZXing::BarcodeFormat::QRCode);
        options.setTryHarder(true);
        options.setTryRotate(true);

        auto result = ZXing::ReadBarcode(imageView, options);

        if (result.isValid())
        {
            return QString::fromStdString(result.text());
        }
        else
        {
            return "未找到二维码或识别失败";
        }
    }
    catch (const std::exception& e)
    {
        qDebug() << "Recognition error:" << e.what();
        return QString("识别错误：%1").arg(e.what());
    }
}

void MainWindow::displayRecognitionResult(const QString& result)
{
    m_resultTextEdit->clear();
    m_resultTextEdit->setPlainText(result);

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