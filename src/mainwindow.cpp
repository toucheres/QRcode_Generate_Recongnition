#include "mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <QTextCodec>
#include <QFileDialog>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QTabWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>
#include <QCameraDevice>
#include <QMediaDevices>

// ZXing includes
#ifdef ZXING_EXPERIMENTAL_API
#include "WriteBarcode.h"
#include "BarcodeFormat.h"
#else
#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "BarcodeFormat.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_tabWidget(nullptr)
    , m_generateWidget(nullptr)
    , m_generateTitleLabel(nullptr)
    , m_textInput(nullptr)
    , m_generateButton(nullptr)
    , m_qrCodeLabel(nullptr)
    , m_recognizeWidget(nullptr)
    , m_recognizeTitleLabel(nullptr)
    , m_selectFileButton(nullptr)
    , m_urlInput(nullptr)
    , m_loadUrlButton(nullptr)
    , m_imageDisplayLabel(nullptr)
    , m_resultTextEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_cameraWidget(nullptr)
    , m_cameraComboBox(nullptr)
    , m_toggleCameraButton(nullptr)
    , m_captureButton(nullptr)
    , m_videoWidget(nullptr)
    , m_cameraStatusLabel(nullptr)
    , m_cameraResultTextEdit(nullptr)
    , m_networkManager(nullptr)
    , m_currentReply(nullptr)
    , m_camera(nullptr)
    , m_imageCapture(nullptr)
    , m_captureSession(nullptr)
    , m_recognitionTimer(nullptr)
    , m_cameraActive(false)
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
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    stopCamera();
    if (m_recognitionTimer) {
        m_recognitionTimer->stop();
    }
}

void MainWindow::setupUI()
{
    // 创建中央控件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #ccc; }"
        "QTabBar::tab { background: #f0f0f0; padding: 10px 20px; margin-right: 2px; }"
        "QTabBar::tab:selected { background: #007ACC; color: white; }"
        "QTabBar::tab:hover { background: #e0e0e0; }"
    );
    
    // 设置标签页
    setupGenerateMode();
    setupRecognizeMode();
    setupCameraRecognition();
    
    m_tabWidget->addTab(m_generateWidget, "生成模式");
    m_tabWidget->addTab(m_recognizeWidget, "识别模式");
    m_tabWidget->addTab(m_cameraWidget, "摄像头识别");
    
    // 连接标签页切换信号
    connect(m_tabWidget, QOverload<int>::of(&QTabWidget::currentChanged), 
            this, &MainWindow::onModeChanged);
    
    mainLayout->addWidget(m_tabWidget);
    
    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);
    
    // 初始化摄像头 - 移到setupUI的最后
    initializeCamera();
}

void MainWindow::setupGenerateMode()
{
    m_generateWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_generateWidget);
    
    // 标题
    m_generateTitleLabel = new QLabel("QR码生成器", m_generateWidget);
    m_generateTitleLabel->setAlignment(Qt::AlignCenter);
    m_generateTitleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");
    
    // 输入框
    m_textInput = new QLineEdit(m_generateWidget);
    m_textInput->setPlaceholderText("请输入要生成二维码的文本（支持中文）...");
    m_textInput->setStyleSheet("QLineEdit { font-size: 14px; padding: 10px; margin: 10px 0; }");
    
    // 生成按钮
    m_generateButton = new QPushButton("生成二维码", m_generateWidget);
    m_generateButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #007ACC; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #005a9e; }");
    
    // 二维码显示区域
    m_qrCodeLabel = new QLabel(m_generateWidget);
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setMinimumSize(350, 350);
    m_qrCodeLabel->setStyleSheet("QLabel { border: 2px dashed #ccc; background-color: #f9f9f9; }");
    m_qrCodeLabel->setText("二维码将显示在这里");
    
    // 连接信号
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateQRCode);
    connect(m_textInput, &QLineEdit::returnPressed, this, &MainWindow::onGenerateQRCode);
    
    // 布局
    layout->addWidget(m_generateTitleLabel);
    layout->addWidget(m_textInput);
    layout->addWidget(m_generateButton, 0, Qt::AlignCenter);
    layout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    layout->addStretch();
}

void MainWindow::setupRecognizeMode()
{
    m_recognizeWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_recognizeWidget);
    
    // 标题
    m_recognizeTitleLabel = new QLabel("QR码识别器", m_recognizeWidget);
    m_recognizeTitleLabel->setAlignment(Qt::AlignCenter);
    m_recognizeTitleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");
    
    // 创建输入区域
    QGroupBox *inputGroup = new QGroupBox("选择图片来源");
    QVBoxLayout *inputLayout = new QVBoxLayout(inputGroup);
    
    // 文件选择
    QHBoxLayout *fileLayout = new QHBoxLayout();
    m_selectFileButton = new QPushButton("选择本地图片", m_recognizeWidget);
    m_selectFileButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; }");
    fileLayout->addWidget(m_selectFileButton);
    fileLayout->addStretch();
    
    // URL输入
    QHBoxLayout *urlLayout = new QHBoxLayout();
    QLabel *urlLabel = new QLabel("或输入图片URL:");
    m_urlInput = new QLineEdit(m_recognizeWidget);
    m_urlInput->setPlaceholderText("https://example.com/qrcode.png");
    m_loadUrlButton = new QPushButton("加载", m_recognizeWidget);
    m_loadUrlButton->setStyleSheet("QPushButton { font-size: 14px; padding: 8px 16px; background-color: #17a2b8; color: white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #138496; }");
    m_loadUrlButton->setEnabled(false);
    
    urlLayout->addWidget(urlLabel);
    urlLayout->addWidget(m_urlInput, 1);
    urlLayout->addWidget(m_loadUrlButton);
    
    inputLayout->addLayout(fileLayout);
    inputLayout->addLayout(urlLayout);
    
    // 创建显示区域
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // 图片显示
    QGroupBox *imageGroup = new QGroupBox("图片预览");
    QVBoxLayout *imageLayout = new QVBoxLayout(imageGroup);
    m_imageDisplayLabel = new QLabel();
    m_imageDisplayLabel->setAlignment(Qt::AlignCenter);
    m_imageDisplayLabel->setMinimumSize(300, 300);
    m_imageDisplayLabel->setStyleSheet("QLabel { border: 2px dashed #ccc; background-color: #f9f9f9; }");
    m_imageDisplayLabel->setText("选择图片后将在此显示");
    m_imageDisplayLabel->setScaledContents(true);
    imageLayout->addWidget(m_imageDisplayLabel);
    
    // 结果显示
    QGroupBox *resultGroup = new QGroupBox("识别结果");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
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
    QVBoxLayout *mainLayout = new QVBoxLayout(m_cameraWidget);
    
    // 标题
    QLabel *titleLabel = new QLabel("摄像头QR码识别", m_cameraWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");
    
    // 控制区域
    QGroupBox *controlGroup = new QGroupBox("摄像头控制");
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    
    // 摄像头选择
    QHBoxLayout *cameraSelectLayout = new QHBoxLayout();
    QLabel *cameraLabel = new QLabel("选择摄像头:");
    m_cameraComboBox = new QComboBox();
    
    // 添加刷新按钮
    QPushButton *refreshButton = new QPushButton("刷新");
    refreshButton->setStyleSheet("QPushButton { font-size: 12px; padding: 5px 10px; background-color: #6c757d; color: white; border: none; border-radius: 3px; } QPushButton:hover { background-color: #5a6268; }");
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::updateCameraList);
    
    cameraSelectLayout->addWidget(cameraLabel);
    cameraSelectLayout->addWidget(m_cameraComboBox, 1);
    cameraSelectLayout->addWidget(refreshButton);
    cameraSelectLayout->addStretch();
    
    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_toggleCameraButton = new QPushButton("启动摄像头");
    m_toggleCameraButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; }");
    
    m_captureButton = new QPushButton("拍照识别");
    m_captureButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #007ACC; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #005a9e; }");
    m_captureButton->setEnabled(false);
    
    buttonLayout->addWidget(m_toggleCameraButton);
    buttonLayout->addWidget(m_captureButton);
    buttonLayout->addStretch();
    
    controlLayout->addLayout(cameraSelectLayout);
    controlLayout->addLayout(buttonLayout);
    
    // 创建水平分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // 视频显示区域
    QGroupBox *videoGroup = new QGroupBox("摄像头画面");
    QVBoxLayout *videoLayout = new QVBoxLayout(videoGroup);
    m_videoWidget = new QVideoWidget();
    m_videoWidget->setMinimumSize(400, 300);
    m_videoWidget->setStyleSheet("QVideoWidget { border: 2px solid #ccc; background-color: #000; }");
    videoLayout->addWidget(m_videoWidget);
    
    // 结果显示区域
    QGroupBox *resultGroup = new QGroupBox("识别结果");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    m_cameraResultTextEdit = new QTextEdit();
    m_cameraResultTextEdit->setPlaceholderText("实时识别结果将显示在这里...\n启动摄像头后会自动尝试识别画面中的二维码");
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
    connect(m_cameraComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onCameraChanged);
    
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
    
    if (m_availableCameras.isEmpty()) {
        m_cameraComboBox->addItem("未找到摄像头");
        m_toggleCameraButton->setEnabled(false);
        m_cameraStatusLabel->setText("未检测到摄像头设备");
        
        // 检查权限和驱动问题
        qDebug() << "No cameras found. Possible issues:";
        qDebug() << "1. Camera permission not granted";
        qDebug() << "2. Camera driver not installed";
        qDebug() << "3. Camera already in use by another application";
        
        // 显示详细错误信息给用户
        QTimer::singleShot(100, this, [this]() {
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
    for (int i = 0; i < m_availableCameras.size(); ++i) {
        const QCameraDevice &cameraDevice = m_availableCameras.at(i);
        QString deviceInfo = QString("%1 (%2)")
                           .arg(cameraDevice.description())
                           .arg(cameraDevice.id());
        
        m_cameraComboBox->addItem(deviceInfo);
        
        qDebug() << "Camera" << i << ":" 
                 << "Description:" << cameraDevice.description()
                 << "ID:" << cameraDevice.id()
                 << "Default:" << cameraDevice.isDefault();
    }
    
    // 选择默认摄像头
    for (int i = 0; i < m_availableCameras.size(); ++i) {
        if (m_availableCameras.at(i).isDefault()) {
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
    
    if (m_availableCameras.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到可用的摄像头！\n请检查摄像头连接和权限设置。");
        return;
    }
    
    try {
        // 停止之前的摄像头
        if (m_camera) {
            m_camera->stop();
            m_camera->deleteLater();
            m_camera = nullptr;
        }
        
        int selectedIndex = m_cameraComboBox->currentIndex();
        if (selectedIndex < 0 || selectedIndex >= m_availableCameras.size()) {
            QMessageBox::warning(this, "错误", "无效的摄像头选择！");
            return;
        }
        
        const QCameraDevice &selectedCamera = m_availableCameras.at(selectedIndex);
        qDebug() << "Selected camera:" << selectedCamera.description();
        
        // 创建摄像头
        m_camera = new QCamera(selectedCamera, this);
        
        // 连接错误信号
        connect(m_camera, &QCamera::errorOccurred, this, &MainWindow::onCameraError);
        
        // 连接状态变化信号
        connect(m_camera, &QCamera::activeChanged, this, [this](bool active) {
            qDebug() << "Camera active state changed:" << active;
            if (active) {
                m_cameraStatusLabel->setText("摄像头已启动 - 实时识别中...");
            }
        });
        
        // 设置摄像头到捕获会话
        m_captureSession->setCamera(m_camera);
        
        // 检查摄像头是否可用
        if (!m_camera->isAvailable()) {
            QMessageBox::warning(this, "错误", "选择的摄像头当前不可用！\n可能正被其他应用程序使用。");
            return;
        }
        
        // 启动摄像头
        m_camera->start();
        
        // 等待摄像头启动
        QTimer::singleShot(1000, this, [this]() {
            if (m_camera && m_camera->isActive()) {
                m_cameraActive = true;
                m_toggleCameraButton->setText("停止摄像头");
                m_toggleCameraButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #dc3545; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #c82333; }");
                m_captureButton->setEnabled(true);
                m_cameraComboBox->setEnabled(false);
                
                // 启动实时识别定时器
                m_recognitionTimer->start();
                
                qDebug() << "Camera started successfully";
            } else {
                QMessageBox::warning(this, "错误", "摄像头启动失败！\n请检查摄像头权限和驱动程序。");
            }
        });
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("启动摄像头失败：%1").arg(e.what()));
        qDebug() << "Camera start exception:" << e.what();
    }
}

void MainWindow::stopCamera()
{
    if (m_camera) {
        m_camera->stop();
        m_camera->deleteLater();
        m_camera = nullptr;
    }
    
    m_recognitionTimer->stop();
    
    m_cameraActive = false;
    m_toggleCameraButton->setText("启动摄像头");
    m_toggleCameraButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #28a745; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #218838; }");
    m_captureButton->setEnabled(false);
    m_cameraComboBox->setEnabled(true);
    m_cameraStatusLabel->setText("摄像头已停止");
}

void MainWindow::onCaptureImage()
{
    if (m_imageCapture && m_cameraActive) {
        m_imageCapture->capture();
    }
}

void MainWindow::onCameraChanged(int index)
{
    if (m_cameraActive) {
        // 如果摄像头正在运行，先停止再重新启动
        stopCamera();
        startCamera();
    }
}

void MainWindow::onImageCaptured(int id, const QImage& image)
{
    Q_UNUSED(id)
    
    if (image.isNull()) {
        return;
    }
    
    QString result = recognizeQRCodeFromImage(image);
    
    // 更新结果显示
    m_cameraResultTextEdit->clear();
    if (result.startsWith("未找到") || result.startsWith("识别错误")) {
        m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #dc3545; }");
        m_cameraResultTextEdit->setPlainText(QString("拍照识别结果：%1").arg(result));
    } else {
        m_cameraResultTextEdit->setStyleSheet("QTextEdit { color: #28a745; font-weight: bold; }");
        m_cameraResultTextEdit->setPlainText(QString("拍照识别成功！\n内容：%1").arg(result));
        
        // 播放成功提示音（可选）
        QMessageBox::information(this, "识别成功", QString("识别到二维码内容：\n%1").arg(result));
    }
}

void MainWindow::onCameraError(QCamera::Error error)
{
    QString errorString;
    switch (error) {
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
    if (m_cameraActive && m_imageCapture) {
        recognizeFromVideoFrame();
    }
}

void MainWindow::recognizeFromVideoFrame()
{
    // 这里可以实现从视频流中提取帧进行识别
    // 由于Qt6的VideoFrame API限制，我们使用定时拍照的方式
    if (m_imageCapture && m_cameraActive) {
        // 静默捕获用于实时识别
        m_imageCapture->capture();
    }
}

void MainWindow::onGenerateQRCode()
{
    QString text = m_textInput->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要生成二维码的文本！");
        return;
    }
    
    // 显示输入文本的调试信息
    qDebug() << "Input text:" << text;
    qDebug() << "Text length:" << text.length();
    qDebug() << "UTF-8 bytes:" << text.toUtf8().toHex();
    
    try {
        QPixmap qrPixmap = generateQRCodePixmap(text);
        if (!qrPixmap.isNull()) {
            m_qrCodeLabel->setPixmap(qrPixmap);
            m_qrCodeLabel->setText("");
            
            // 显示成功信息
            QString info = QString("二维码生成成功！\n文本：%1\n长度：%2 字符")
                          .arg(text)
                          .arg(text.length());
            QMessageBox::information(this, "成功", info);
        } else {
            QMessageBox::critical(this, "错误", "生成二维码失败！");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "错误", QString("生成二维码时发生错误：%1").arg(e.what()));
    }
}

QPixmap MainWindow::generateQRCodePixmap(const QString& text)
{
    try {
        // 确保使用UTF-8编码
        QByteArray utf8Data = text.toUtf8();
        std::string stdText = utf8Data.toStdString();
        
        qDebug() << "Generating QR code for UTF-8 text:" << QString::fromUtf8(utf8Data);
        qDebug() << "std::string size:" << stdText.size();
        
#ifdef ZXING_EXPERIMENTAL_API
        // 使用实验性API
        ZXing::CreatorOptions options(ZXing::BarcodeFormat::QRCode);
        options.ecLevel("M"); // 设置错误纠正级别为中等
        
        auto barcode = ZXing::CreateBarcodeFromText(stdText, options);
        if (!barcode.isValid()) {
            qDebug() << "Failed to create barcode";
            return QPixmap();
        }
        
        ZXing::WriterOptions writerOptions;
        writerOptions.sizeHint(300).withQuietZones(true);
        
        auto image = ZXing::WriteBarcodeToImage(barcode, writerOptions);
        
        // 将ZXing Image转换为QPixmap
        QImage qImage(image.data(), image.width(), image.height(), QImage::Format_Grayscale8);
        return QPixmap::fromImage(qImage);
        
#else
        // 使用传统API
        ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QRCode);
        writer.setMargin(2);
        writer.setEccLevel(1); // 错误纠正级别 M
        
        // 设置编码为UTF-8以支持中文字符
        writer.setEncoding(ZXing::CharacterSet::UTF8);
        
        auto bitMatrix = writer.encode(stdText, 300, 300);
        qDebug() << "BitMatrix size:" << bitMatrix.width() << "x" << bitMatrix.height();
        
        return zxingMatrixToQPixmap(bitMatrix);
#endif
        
    } catch (const std::exception& e) {
        qDebug() << "Error generating QR code:" << e.what();
        return QPixmap();
    }
}

QPixmap MainWindow::zxingMatrixToQPixmap(const ZXing::BitMatrix& matrix)
{
    int width = matrix.width();
    int height = matrix.height();
    
    QImage image(width, height, QImage::Format_RGB32);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QRgb color = matrix.get(x, y) ? qRgb(0, 0, 0) : qRgb(255, 255, 255);
            image.setPixel(x, y, color);
        }
    }
    
    return QPixmap::fromImage(image);
}

void MainWindow::onSelectImageFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择二维码图片",
        "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff)"
    );
    
    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        if (pixmap.isNull()) {
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
    if (urlText.isEmpty()) {
        return;
    }
    
    QUrl url(urlText);
    if (!url.isValid()) {
        QMessageBox::warning(this, "错误", "请输入有效的URL地址！");
        return;
    }
    
    // 取消之前的请求
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    m_statusLabel->setText("正在加载图片...");
    m_loadUrlButton->setEnabled(false);
    
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
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
    if (!m_currentReply) {
        return;
    }
    
    m_loadUrlButton->setEnabled(true);
    
    if (m_currentReply->error() != QNetworkReply::NoError) {
        m_statusLabel->setText("加载失败");
        QMessageBox::warning(this, "网络错误", 
                           QString("无法加载图片：%1").arg(m_currentReply->errorString()));
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }
    
    QByteArray imageData = m_currentReply->readAll();
    QPixmap pixmap;
    
    if (!pixmap.loadFromData(imageData)) {
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
    qDebug() << "Mode changed to:" << (index == 0 ? "Generate" : (index == 1 ? "Recognize" : "Camera"));
    
    // 当切换到其他模式时，停止摄像头
    if (index != 2 && m_cameraActive) {
        stopCamera();
    }
}

QString MainWindow::recognizeQRCodeFromPixmap(const QPixmap& pixmap)
{
    return recognizeQRCodeFromImage(pixmap.toImage());
}

QString MainWindow::recognizeQRCodeFromImage(const QImage& image)
{
    try {
        // 转换为ZXing可以处理的格式
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
        
        ZXing::ImageView imageView(rgbImage.bits(), rgbImage.width(), rgbImage.height(), 
                                   ZXing::ImageFormat::RGB, rgbImage.bytesPerLine());
        
        ZXing::ReaderOptions options;
        options.setFormats(ZXing::BarcodeFormat::QRCode);
        options.setTryHarder(true);
        options.setTryRotate(true);
        
        auto result = ZXing::ReadBarcode(imageView, options);
        
        if (result.isValid()) {
            return QString::fromStdString(result.text());
        } else {
            return "未找到二维码或识别失败";
        }
        
    } catch (const std::exception& e) {
        qDebug() << "Recognition error:" << e.what();
        return QString("识别错误：%1").arg(e.what());
    }
}

void MainWindow::displayRecognitionResult(const QString& result)
{
    m_resultTextEdit->clear();
    m_resultTextEdit->setPlainText(result);
    
    if (result.startsWith("未找到") || result.startsWith("识别错误")) {
        m_resultTextEdit->setStyleSheet("QTextEdit { color: #dc3545; }");
        m_statusLabel->setText("识别失败");
    } else {
        m_resultTextEdit->setStyleSheet("QTextEdit { color: #28a745; }");
        m_statusLabel->setText("识别成功");
    }
}

void MainWindow::displaySelectedImage(const QPixmap& pixmap)
{
    // 缩放图片以适应显示区域
    QPixmap scaledPixmap = pixmap.scaled(m_imageDisplayLabel->size(), 
                                        Qt::KeepAspectRatio, 
                                        Qt::SmoothTransformation);
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
    
    for (int i = 0; i < videoInputs.size(); ++i) {
        const QCameraDevice &device = videoInputs.at(i);
        qDebug() << "Device" << i << ":";
        qDebug() << "  Description:" << device.description();
        qDebug() << "  ID:" << device.id();
        qDebug() << "  IsDefault:" << device.isDefault();
        qDebug() << "  Position:" << static_cast<int>(device.position());
        
        // 输出支持的视频格式
        auto formats = device.videoFormats();
        qDebug() << "  Supported formats:" << formats.size();
        for (const auto &format : formats) {
            qDebug() << "    Resolution:" << format.resolution() 
                     << "FPS:" << format.minFrameRate() << "-" << format.maxFrameRate();
        }
    }
    
    qDebug() << "=== End Camera Debug Information ===";
}

void MainWindow::onToggleCamera()
{
    if (m_cameraActive) {
        stopCamera();
    } else {
        startCamera();
    }
}