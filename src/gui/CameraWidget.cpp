#include "gui/CameraWidget.h"
#include "utils/AppUtils.h"
#include "utils/AppSettings.h"
#include <QApplication>
#include <QAudioOutput>
#include <QCameraDevice>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMediaDevices>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStringConverter>
#include <QTextStream>

CameraWidget::CameraWidget(QWidget* parent)
    : BaseWidget(parent), m_recognizer(new QRCodeRecognizer(this)), m_camera(nullptr),
      m_captureSession(new QMediaCaptureSession(this)), m_videoWidget(new QVideoWidget(this)),
      m_imageCapture(new QImageCapture(this)), m_videoSink(new QVideoSink(this)),
      m_overlayLabel(new QLabel(m_videoWidget)), m_recognitionTimer(new QTimer(this)),
      m_overlayTimer(new QTimer(this)), m_cameraActive(false), m_realtimeRecognition(true),
      m_detectionCount(0), m_lastImageSize(1280, 720) // 默认图像尺寸
{
    setupUI();
    setupCamera();
    //     1.先设置windowFlag，setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::SubWindow);
    // 设置叠加层
    m_overlayLabel->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::SubWindow);
    m_overlayLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_overlayLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_overlayLabel->setAutoFillBackground(false);
    m_overlayLabel->hide(); // 默认隐藏

    // 连接识别器信号 - 修复参数不匹配问题
    connect(m_recognizer, &QRCodeRecognizer::recognitionCompleted, this,
            [this](const QRCodeRecognizer::RecognitionResult& result, int requestId)
            {
                Q_UNUSED(requestId) // 忽略请求ID
                onRecognitionResult(result);
            });
    QTimer* cout = new QTimer(this);
    cout->setInterval(1000);
    cout->callOnTimeout(
        [this]()
        {
            // 获取CameraWidget的全局位置
            QPoint cameraGlobalPos = this->mapToGlobal(QPoint(0, 0));
            // 获取视频控件的全局位置
            QPoint videoGlobalPos = m_videoWidget->mapToGlobal(QPoint(0, 0));
            // 获取叠加层的全局位置
            QPoint overlayGlobalPos = m_overlayLabel->mapToGlobal(QPoint(0, 0));

            // 设置叠加层全局位置与视频控件一致
            m_overlayLabel->move(videoGlobalPos);
            m_overlayLabel->resize(m_videoWidget->size());

            qDebug() << "Camera global pos:" << cameraGlobalPos.x() << cameraGlobalPos.y()
                     << "Video global pos:" << videoGlobalPos.x() << videoGlobalPos.y()
                     << "Overlay global pos:" << overlayGlobalPos.x() << overlayGlobalPos.y()
                     << "Overlay local pos:" << m_overlayLabel->x() << m_overlayLabel->y()
                     << "Overlay adjusted to video position";
        });
    cout->start(); // 启动定时器
    // 连接识别失败信号用于调试
    connect(m_recognizer, &QRCodeRecognizer::recognitionFailed, this,
            [this](const QString& error, int requestId)
            {
                static int failCount = 0;
                failCount++;
                if (failCount % 10 == 0)
                { // 每10次失败打印一次
                    qDebug() << "Recognition failed:" << failCount << "Error:" << error
                             << "RequestID:" << requestId;
                }
            });

    // 连接定时器信号
    connect(m_recognitionTimer, &QTimer::timeout, this, &CameraWidget::onRecognitionTimer);

    // 添加捕获会话状态监控
    connect(m_captureSession, &QMediaCaptureSession::videoOutputChanged, this,
            []() { qDebug() << "Video output changed"; });

    // 添加权限检查（Windows可能需要）
    qDebug() << "Checking camera permissions...";
    auto devices = QMediaDevices::videoInputs();
    if (!devices.isEmpty())
    {
        qDebug() << "Camera devices available:" << devices.size();
    }
    else
    {
        qDebug() << "No camera devices found - may need permissions";
    }

    // 设置识别定时器
    m_recognitionTimer->setSingleShot(false);
    m_recognitionTimer->setInterval(500); // 默认500ms识别一次，与滑块初始值同步

    // 设置叠加层计时器
    m_overlayTimer->setSingleShot(true); // 单次触发
    connect(m_overlayTimer, &QTimer::timeout, this,
            [this]()
            {
                if (m_overlayLabel)
                {
                    m_overlayLabel->hide();
                    qDebug() << "Overlay hidden by timer";
                }
            });

    qDebug() << "CameraWidget initialized";
}

CameraWidget::~CameraWidget()
{
    stopCamera();
}

void CameraWidget::startCamera()
{
    if (!m_camera)
    {
        emit cameraError("未找到可用的摄像头设备");
        return;
    }

    qDebug() << "Starting camera...";
    qDebug() << "Video widget size:" << m_videoWidget->size();
    qDebug() << "Video widget visible:" << m_videoWidget->isVisible();

    try
    {
        // 确保摄像头已连接到会话
        if (m_captureSession->camera() != m_camera)
        {
            m_captureSession->setCamera(m_camera);
        }

        // 确保视频输出已设置
        if (m_captureSession->videoOutput() != m_videoWidget)
        {
            qDebug() << "Setting video output...";
            m_captureSession->setVideoOutput(m_videoWidget);
        }

        // 检查摄像头状态
        qDebug() << "Camera active before start:" << m_camera->isActive();
        qDebug() << "Camera error:" << m_camera->error();

        // 启动摄像头
        m_camera->start();
        m_cameraActive = true;
        m_cameraToggleButton->setText("停止摄像头");
        m_captureButton->setEnabled(true);
        m_cameraStatusLabel->setText("正在启动摄像头...");

        qDebug() << "Camera started successfully";
        qDebug() << "Camera active after start:" << m_camera->isActive();

        // 启动实时识别定时器（如果启用）
        if (m_realtimeRecognitionCheckBox->isChecked())
        {
            m_recognitionTimer->start();
            qDebug() << "Recognition timer started";
        }

        // 强制更新视频widget
        m_videoWidget->update();
        m_videoWidget->repaint();
    }
    catch (const std::exception& e)
    {
        qDebug() << "Camera start error:" << e.what();
        emit cameraError(QString("启动摄像头失败: %1").arg(e.what()));
        m_cameraActive = false;
        m_cameraToggleButton->setText("启动摄像头");
        m_captureButton->setEnabled(false);
        m_cameraStatusLabel->setText("摄像头启动失败");
    }
}

void CameraWidget::stopCamera()
{
    if (m_camera)
    {
        m_camera->stop();
    }

    m_recognitionTimer->stop();
    m_cameraActive = false;
    m_cameraToggleButton->setText("启动摄像头");
    m_captureButton->setEnabled(false);
    m_cameraStatusLabel->setText("摄像头已停止");
}

bool CameraWidget::isCameraAvailable() const
{
    return !QMediaDevices::videoInputs().isEmpty();
}

void CameraWidget::onRecognitionResult(const QRCodeRecognizer::RecognitionResult& result)
{
    static int resultCount = 0;
    resultCount++;

    qDebug() << "Recognition result received:" << resultCount << "Valid:" << result.isValid;

    if (result.isValid)
    {
        qDebug() << "QR Code detected! Text:" << result.text << "Format:" << result.format;

        // 避免重复检测相同内容
        QDateTime now = QDateTime::currentDateTime();
        if (result.text == m_lastDetectedText && m_lastDetectionTime.secsTo(now) < 2)
        {
            qDebug() << "Ignoring duplicate detection within 2 seconds";
            return; // 2秒内的重复检测忽略
        }

        m_lastDetectedText = result.text;
        m_lastDetectionTime = now;

        addResultToHistory(result);

        if (m_showOverlayCheckBox->isChecked())
        {
            // 先隐藏之前的叠加层，再绘制新的
            m_overlayTimer->stop();
            m_overlayLabel->hide();
            drawDetectionOverlay(result);
        }

        emit qrCodeDetected(result.text, result);

        // 自动保存到剪贴板
        QApplication::clipboard()->setText(result.text);

        // 尝试自动打开URL（如果启用了该功能）
        AppSettings::instance().tryAutoOpenUrl(result.text);

        m_detectionCount++;
        m_detectionCountLabel->setText(QString("检测次数: %1").arg(m_detectionCount));

        qDebug() << "QR Code processed successfully, detection count:" << m_detectionCount;
    }
    else
    {
        // 仅偶尔打印无效结果，避免日志泛滥
        if (resultCount % 50 == 0)
        {
            qDebug() << "Invalid recognition result, total attempts:" << resultCount;
        }
    }
}

void CameraWidget::onCameraToggleClicked()
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

void CameraWidget::onCaptureClicked()
{
    qDebug() << "Manual capture clicked";
    qDebug() << "Camera active:" << m_cameraActive;
    qDebug() << "Camera object:" << (m_camera != nullptr);
    qDebug() << "ImageCapture ready:" << (m_imageCapture && m_imageCapture->isReadyForCapture());

    if (m_camera)
    {
        qDebug() << "Camera is active:" << m_camera->isActive();
    }

    if (!m_cameraActive || !m_camera || !m_camera->isActive())
    {
        QMessageBox::warning(this, "警告", "摄像头未启动！\n请先启动摄像头。");
        return;
    }

    if (!m_imageCapture || !m_imageCapture->isReadyForCapture())
    {
        QMessageBox::warning(this, "警告", "图像捕获设备未准备就绪！\n请稍后再试。");
        return;
    }

    qDebug() << "Starting manual image capture...";

    // 设置一个临时的连接来处理手动捕获的结果
    static QMetaObject::Connection manualCaptureConnection;

    // 断开之前的连接（如果存在）
    if (manualCaptureConnection)
    {
        disconnect(manualCaptureConnection);
    }

    // 创建新的连接来处理这次手动捕获
    manualCaptureConnection =
        connect(m_imageCapture, &QImageCapture::imageCaptured, this,
                [this](int id, const QImage& image)
                {
                    Q_UNUSED(id)
                    qDebug() << "Manual capture completed - Image null:" << image.isNull()
                             << "Size:" << image.size();

                    if (!image.isNull())
                    {
                        QRCodeRecognizer::RecognitionConfig config;
                        config.tryHarder = true; // 手动捕获时使用更高精度
                        config.tryRotate = true;
                        config.fastMode = false;

                        qDebug() << "Starting manual synchronous recognition...";
                        auto result = m_recognizer->recognizeSync(image, config);
                        qDebug() << "Manual recognition result - valid:" << result.isValid;

                        if (result.isValid)
                        {
                            qDebug() << "Manual recognition successful:" << result.text;
                            onRecognitionResult(result);
                        }
                        else
                        {
                            qDebug() << "Manual recognition failed";
                            QMessageBox::information(this, "结果", "当前画面中未检测到二维码");
                        }
                    }
                    else
                    {
                        qDebug() << "Manual capture returned null image";
                        QMessageBox::warning(this, "警告", "图像捕获失败！");
                    }

                    // 断开这个临时连接
                    disconnect(manualCaptureConnection);
                    manualCaptureConnection = QMetaObject::Connection();
                });

    // 执行捕获
    m_imageCapture->capture();
}

void CameraWidget::onClearHistoryClicked()
{
    m_historyTextEdit->clear();
    m_detectionHistory.clear();
    m_detectionCount = 0;
    m_detectionCountLabel->setText("检测次数: 0");
}

void CameraWidget::onSaveHistoryClicked()
{
    if (m_detectionHistory.isEmpty())
    {
        QMessageBox::warning(this, "警告", "没有检测历史可保存！");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "保存检测历史",
        QString("camera_detection_%1.txt")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);

            out << "摄像头二维码检测历史\n";
            out << "生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                << "\n";
            out << "检测总数: " << m_detectionHistory.size() << "\n\n";

            for (int i = 0; i < m_detectionHistory.size(); ++i)
            {
                const auto& result = m_detectionHistory[i];
                out << "=== 检测 " << (i + 1) << " ===\n";
                out << "内容: " << result.text << "\n";
                out << "格式: " << result.format << "\n";
                out << "置信度: " << QString::number(result.confidence, 'f', 2) << "\n\n";
            }
        }
    }
}

void CameraWidget::onCameraDeviceChanged()
{
    int index = m_deviceCombo->currentIndex();
    if (index >= 0)
    {
        auto devices = QMediaDevices::videoInputs();
        if (index < devices.size())
        {
            bool wasActive = m_cameraActive;
            if (wasActive)
            {
                stopCamera();
            }

            // 重新创建摄像头
            if (m_camera)
            {
                m_captureSession->setCamera(nullptr); // 先断开连接
                delete m_camera;
                m_camera = nullptr;
            }

            // 创建新的摄像头实例
            m_camera = new QCamera(devices[index], this);
            m_captureSession->setCamera(m_camera);

            // 重新连接信号
            connect(m_camera, &QCamera::errorOccurred, this, &CameraWidget::onCameraErrorOccurred);
            connect(m_camera, &QCamera::activeChanged, this,
                    [](bool active) { qDebug() << "Camera active changed:" << active; });

            updateResolutions();

            // 如果之前是活动状态，重新启动
            if (wasActive)
            {
                startCamera();
            }
            else
            {
                m_cameraStatusLabel->setText("摄像头已准备就绪");
            }
        }
    }
}

void CameraWidget::onResolutionChanged()
{
    // 实现分辨率更改逻辑
    // 这需要根据具体的摄像头API来实现
}

void CameraWidget::onRecognitionSettingsChanged()
{
    m_realtimeRecognition = m_realtimeRecognitionCheckBox->isChecked();

    if (m_realtimeRecognition && m_cameraActive)
    {
        int interval = m_recognitionIntervalSlider->value() * 100; // 转换为毫秒
        m_recognitionTimer->setInterval(interval);
        m_recognitionTimer->start();
        m_intervalLabel->setText(QString("%1ms").arg(interval));
    }
    else
    {
        m_recognitionTimer->stop();
    }
}

void CameraWidget::onVideoFrameChanged(const QVideoFrame& frame)
{
    static int frameUpdateCount = 0;
    frameUpdateCount++;

    if (frameUpdateCount % 60 == 0)
    { // 每60帧打印一次（约每2秒）
        qDebug() << "Video frame updated:" << frameUpdateCount << "Valid:" << frame.isValid()
                 << "Size:" << frame.size();
    }

    m_currentFrame = frame;

    // 确保帧是有效的并且已映射
    if (frame.isValid() && frameUpdateCount == 1)
    {
        qDebug() << "First valid frame received!"
                 << "Format:" << frame.pixelFormat() << "Size:" << frame.size();
    }
}

void CameraWidget::onRecognitionTimer()
{
    // 在新的方法中，我们不依赖videoFrame，而是主动捕获图像
    if (m_cameraActive && m_realtimeRecognition && m_imageCapture)
    {
        static int timerCount = 0;
        timerCount++;
        if (timerCount % 10 == 0)
        { // 每10次打印一次
            qDebug() << "Recognition timer triggered:" << timerCount
                     << "Camera active:" << m_cameraActive
                     << "Realtime enabled:" << m_realtimeRecognition;
        }

        // 主动捕获图像进行识别
        if (m_imageCapture->isReadyForCapture())
        {
            m_imageCapture->capture();
        }
        else
        {
            if (timerCount % 20 == 0)
            {
                qDebug() << "Image capture not ready";
            }
        }
    }
    else
    {
        static int skipCount = 0;
        skipCount++;
        if (skipCount % 20 == 0)
        { // 每20次跳过打印一次
            qDebug() << "Skipping recognition - Camera active:" << m_cameraActive
                     << "Realtime enabled:" << m_realtimeRecognition
                     << "ImageCapture available:" << (m_imageCapture != nullptr);
        }
    }
}

void CameraWidget::onCameraErrorOccurred()
{
    if (m_camera)
    {
        emit cameraError(QString("摄像头错误: %1").arg(m_camera->errorString()));
    }
}

void CameraWidget::setupUI()
{
    setWindowTitle("摄像头二维码识别");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);

    // 左侧：摄像头预览
    QWidget* cameraWidget = new QWidget();
    cameraWidget->setMinimumWidth(480);
    QVBoxLayout* cameraLayout = createVBoxLayout(cameraWidget);

    cameraLayout->addWidget(createLabel("摄像头预览:"));

    m_videoWidget->setMinimumSize(640, 480);
    m_videoWidget->setStyleSheet("QVideoWidget {"
                                 "    border: 2px solid #cccccc;"
                                 "    border-radius: 8px;"
                                 "    background-color: #000000;"
                                 "}");

    // 确保视频widget可见并具有适当的属性
    m_videoWidget->show();
    m_videoWidget->setAttribute(Qt::WA_NativeWindow, true);
    m_videoWidget->setAttribute(Qt::WA_PaintOnScreen, true);

    qDebug() << "Video widget size:" << m_videoWidget->size();
    qDebug() << "Video widget visible:" << m_videoWidget->isVisible();

    cameraLayout->addWidget(m_videoWidget);

    // 设置叠加层位置与视频控件重叠
    m_overlayLabel->setParent(m_videoWidget);
    m_overlayLabel->resize(m_videoWidget->size());
    m_overlayLabel->move(0, 0);
    m_overlayLabel->raise(); // 确保叠加层在最上面
    m_overlayLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_overlayLabel->setScaledContents(false);

    // 摄像头状态和控制
    m_cameraStatusLabel = createLabel("摄像头未启动");
    // 根据主题设置状态标签颜色
    QPalette palette = QApplication::palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    QString statusColor = isDarkTheme ? "#cccccc" : "#666666";
    m_cameraStatusLabel->setStyleSheet(QString("color: %1; font-size: 10pt;").arg(statusColor));
    cameraLayout->addWidget(m_cameraStatusLabel);

    QHBoxLayout* cameraControlLayout = createHBoxLayout(nullptr, 0);
    m_cameraToggleButton = createButton("启动摄像头");
    m_captureButton = createButton("手动识别");
    m_captureButton->setEnabled(false);
    cameraControlLayout->addWidget(m_cameraToggleButton);
    cameraControlLayout->addWidget(m_captureButton);
    cameraControlLayout->addStretch();
    cameraLayout->addLayout(cameraControlLayout);

    // 右侧：设置和结果
    QWidget* rightWidget = new QWidget();
    rightWidget->setMaximumWidth(350);
    QVBoxLayout* rightLayout = createVBoxLayout(rightWidget);

    // 摄像头设置
    m_cameraGroup = createGroupBox("摄像头设置");
    QVBoxLayout* cameraGroupLayout = createVBoxLayout(m_cameraGroup, 10, 8);

    cameraGroupLayout->addWidget(createLabel("设备:"));
    m_deviceCombo = new QComboBox();
    cameraGroupLayout->addWidget(m_deviceCombo);

    cameraGroupLayout->addWidget(createLabel("分辨率:"));
    m_resolutionCombo = new QComboBox();
    cameraGroupLayout->addWidget(m_resolutionCombo);

    rightLayout->addWidget(m_cameraGroup);

    // 识别设置
    m_recognitionGroup = createGroupBox("识别设置");
    QVBoxLayout* recognitionLayout = createVBoxLayout(m_recognitionGroup, 10, 8);

    m_realtimeRecognitionCheckBox = new QCheckBox("实时识别");
    m_realtimeRecognitionCheckBox->setChecked(true);
    recognitionLayout->addWidget(m_realtimeRecognitionCheckBox);

    m_showOverlayCheckBox = new QCheckBox("显示检测框");
    m_showOverlayCheckBox->setChecked(true);
    recognitionLayout->addWidget(m_showOverlayCheckBox);

    m_autoSaveCheckBox = new QCheckBox("自动保存到剪贴板");
    m_autoSaveCheckBox->setChecked(true);
    recognitionLayout->addWidget(m_autoSaveCheckBox);

    // 识别间隔设置
    QHBoxLayout* intervalLayout = createHBoxLayout(nullptr, 0);
    intervalLayout->addWidget(createLabel("识别间隔:"));
    m_recognitionIntervalSlider = new QSlider(Qt::Horizontal);
    m_recognitionIntervalSlider->setRange(1, 20); // 100ms - 2000ms
    m_recognitionIntervalSlider->setValue(5);     // 默认500ms
    intervalLayout->addWidget(m_recognitionIntervalSlider);
    m_intervalLabel = createLabel("500ms");
    m_intervalLabel->setMinimumWidth(50);
    intervalLayout->addWidget(m_intervalLabel);
    recognitionLayout->addLayout(intervalLayout);

    rightLayout->addWidget(m_recognitionGroup);

    // 检测结果
    m_resultsGroup = createGroupBox("检测历史");
    QVBoxLayout* resultsLayout = createVBoxLayout(m_resultsGroup, 10, 8);

    m_detectionCountLabel = createLabel("检测次数: 0");
    // 根据主题设置计数标签颜色
    QString countColor = isDarkTheme ? "#4CAF50" : "#2196F3";
    m_detectionCountLabel->setStyleSheet(QString("font-weight: bold; color: %1;").arg(countColor));
    resultsLayout->addWidget(m_detectionCountLabel);

    m_historyTextEdit = new QTextEdit();
    m_historyTextEdit->setMaximumHeight(200);
    m_historyTextEdit->setReadOnly(true);
    m_historyTextEdit->setPlaceholderText("检测到的二维码将显示在这里...");
    resultsLayout->addWidget(m_historyTextEdit);

    QHBoxLayout* historyButtonLayout = createHBoxLayout(nullptr, 0);
    m_clearHistoryButton = createButton("清空历史");
    m_saveHistoryButton = createButton("保存历史");
    historyButtonLayout->addWidget(m_clearHistoryButton);
    historyButtonLayout->addWidget(m_saveHistoryButton);
    resultsLayout->addLayout(historyButtonLayout);

    rightLayout->addWidget(m_resultsGroup);
    rightLayout->addStretch();

    // 添加到主布局
    mainLayout->addWidget(cameraWidget);
    mainLayout->addWidget(rightWidget);

    // 连接信号
    connect(m_cameraToggleButton, &QPushButton::clicked, this,
            &CameraWidget::onCameraToggleClicked);
    connect(m_captureButton, &QPushButton::clicked, this, &CameraWidget::onCaptureClicked);
    connect(m_clearHistoryButton, &QPushButton::clicked, this,
            &CameraWidget::onClearHistoryClicked);
    connect(m_saveHistoryButton, &QPushButton::clicked, this, &CameraWidget::onSaveHistoryClicked);
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &CameraWidget::onCameraDeviceChanged);
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &CameraWidget::onResolutionChanged);
    connect(m_realtimeRecognitionCheckBox, &QCheckBox::toggled, this,
            &CameraWidget::onRecognitionSettingsChanged);
    connect(m_recognitionIntervalSlider, &QSlider::valueChanged, this,
            &CameraWidget::onRecognitionSettingsChanged);
}

void CameraWidget::setupCamera()
{
    qDebug() << "Setting up camera...";

    // 在Qt 6中，我们需要选择使用QVideoWidget还是QVideoSink
    // 为了获取视频帧进行识别，我们使用QVideoSink，然后手动显示

    // 首先设置视频sink用于获取帧数据
    m_captureSession->setVideoSink(m_videoSink);

    // 然后设置视频输出用于显示 - 这会覆盖sink设置，但我们稍后处理
    m_captureSession->setVideoOutput(m_videoWidget);

    // 设置图像捕获
    m_captureSession->setImageCapture(m_imageCapture);

    qDebug() << "Capture session configured";
    qDebug() << "Video output:" << m_captureSession->videoOutput();
    qDebug() << "Video sink:" << m_captureSession->videoSink();

    updateCameraDevices();

    if (isCameraAvailable())
    {
        // 立即初始化第一个摄像头设备
        auto devices = QMediaDevices::videoInputs();
        if (!devices.isEmpty())
        {
            qDebug() << "Available cameras:" << devices.size();
            for (const auto& device : devices)
            {
                qDebug() << "Camera:" << device.description() << "ID:" << device.id();
            }

            m_camera = new QCamera(devices[0], this);
            m_captureSession->setCamera(m_camera);

            // 连接摄像头错误信号
            connect(m_camera, &QCamera::errorOccurred, this, &CameraWidget::onCameraErrorOccurred);
            connect(m_camera, &QCamera::activeChanged, this,
                    [this](bool active)
                    {
                        qDebug() << "Camera active changed:" << active;
                        if (active)
                        {
                            m_cameraStatusLabel->setText("摄像头运行中...");
                        }
                    });

            // 重要：由于QVideoWidget和QVideoSink冲突，我们使用ImageCapture获取帧
            connect(m_imageCapture, &QImageCapture::imageCaptured, this,
                    [this](int id, const QImage& image)
                    {
                        Q_UNUSED(id)
                        static int captureCount = 0;
                        captureCount++;
                        if (captureCount % 10 == 0)
                        {
                            qDebug()
                                << "Image captured:" << captureCount << "Size:" << image.size();
                        }

                        if (!image.isNull())
                        {
                            // 创建一个假的QVideoFrame来保持接口一致性
                            // 注意：这里我们直接处理QImage
                            processVideoImage(image);
                        }
                    });

            updateResolutions();
            m_cameraStatusLabel->setText("摄像头已准备就绪");
            qDebug() << "Camera setup completed";
        }
    }
    else
    {
        qDebug() << "No camera devices available";
        m_cameraStatusLabel->setText("未找到可用的摄像头设备");
        m_cameraToggleButton->setEnabled(false);
    }
}

void CameraWidget::updateCameraDevices()
{
    m_deviceCombo->clear();

    auto devices = QMediaDevices::videoInputs();
    for (const auto& device : devices)
    {
        m_deviceCombo->addItem(device.description());
    }

    if (devices.isEmpty())
    {
        m_deviceCombo->addItem("未找到摄像头设备");
        m_deviceCombo->setEnabled(false);
    }
}

void CameraWidget::updateResolutions()
{
    m_resolutionCombo->clear();
    m_resolutionCombo->addItem("640x480");
    m_resolutionCombo->addItem("800x600");
    m_resolutionCombo->addItem("1024x768");
    m_resolutionCombo->addItem("1280x720");
    m_resolutionCombo->addItem("1920x1080");
}

void CameraWidget::processVideoFrame(const QVideoFrame& frame)
{
    QImage image = frame.toImage();
    if (!image.isNull())
    {
        processVideoImage(image);
    }
    else
    {
        static int nullCount = 0;
        nullCount++;
        if (nullCount % 50 == 0)
        {
            qDebug() << "Null image from video frame:" << nullCount;
        }
    }
}

void CameraWidget::processVideoImage(const QImage& image)
{
    if (!image.isNull())
    {
        static int processCount = 0;
        processCount++;

        // 更新最后处理的图像尺寸
        m_lastImageSize = image.size();

        if (processCount % 20 == 0)
        { // 每20帧打印一次详细信息
            qDebug() << "Processing video image:" << processCount << "Image size:" << image.size()
                     << "Image format:" << image.format() << "Image bytes:" << image.sizeInBytes();
        }

        QRCodeRecognizer::RecognitionConfig config;
        config.tryHarder = false; // 实时识别使用快速模式
        config.tryRotate = true;
        config.fastMode = true;
        config.maxSymbols = 1;

        // 异步识别以避免阻塞UI
        static int requestId = 0;
        requestId++;

        if (processCount % 20 == 0)
        {
            qDebug() << "Sending recognition request:" << requestId;
        }

        m_recognizer->recognizeAsync(image, requestId, config);
    }
}

void CameraWidget::addResultToHistory(const QRCodeRecognizer::RecognitionResult& result)
{
    // 检查是否已存在相同的内容（自动去重）
    bool isDuplicate = false;
    for (const auto& existingResult : m_detectionHistory)
    {
        if (existingResult.text == result.text && existingResult.format == result.format)
        {
            isDuplicate = true;
            break;
        }
    }

    // 如果不是重复内容，添加到历史记录
    if (!isDuplicate)
    {
        m_detectionHistory.append(result);

        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        // 检测系统主题以调整HTML样式
        QPalette palette = QApplication::palette();
        bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
        
        QString bgColor = isDarkTheme ? "#2d5a2d" : "#e8f5e8";
        QString borderColor = isDarkTheme ? "#4CAF50" : "#4CAF50";
        QString textColor = isDarkTheme ? "#ffffff" : "#000000";
        
        QString resultHtml =
            QString("<div style='margin: 5px 0; padding: 8px; background-color: "
                    "%4; border-radius: 4px; border-left: 4px solid %5; color: %6;'>"
                    "<b>[%1]</b> 检测到二维码（新）<br/>"
                    "<b>内容:</b> %2<br/>"
                    "<b>格式:</b> %3"
                    "</div>")
                .arg(timestamp)
                .arg(result.text.toHtmlEscaped())
                .arg(result.format)
                .arg(bgColor, borderColor, textColor);

        m_historyTextEdit->append(resultHtml);

        // 自动滚动到底部
        QTextCursor cursor = m_historyTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_historyTextEdit->setTextCursor(cursor);
    }
    else
    {
        // 重复内容，只在界面提示
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        
        // 检测系统主题以调整HTML样式
        QPalette palette = QApplication::palette();
        bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
        
        QString bgColor = isDarkTheme ? "#404040" : "#f0f0f0";
        QString borderColor = isDarkTheme ? "#999999" : "#999999";
        QString textColor = isDarkTheme ? "#cccccc" : "#666666";
        
        QString resultHtml = QString("<div style='margin: 2px 0; padding: 4px; background-color: "
                                     "%4; border-radius: 4px; border-left: 2px solid %5; color: %6;'>"
                                     "<small><b>[%1]</b> 重复检测到相同内容</small>"
                                     "</div>")
                                 .arg(timestamp)
                                 .arg(bgColor, borderColor, textColor);

        m_historyTextEdit->append(resultHtml);

        // 自动滚动到底部
        QTextCursor cursor = m_historyTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_historyTextEdit->setTextCursor(cursor);
    }
}
void CameraWidget::drawDetectionOverlay(const QRCodeRecognizer::RecognitionResult& result)
{
    if (!result.isValid || !m_showOverlayCheckBox->isChecked())
    {
        m_overlayLabel->hide();
        return;
    }

    // 创建透明背景的QPixmap用于绘制轮廓
    QPixmap overlayPixmap(m_videoWidget->size());
    overlayPixmap.fill(Qt::transparent);

    // 确保QPixmap有alpha通道
    if (overlayPixmap.hasAlphaChannel())
    {
        qDebug() << "Overlay pixmap has alpha channel";
    }
    else
    {
        qDebug() << "Creating pixmap with alpha channel";
        overlayPixmap = QPixmap(m_videoWidget->size());
        overlayPixmap.fill(QColor(0, 0, 0, 0)); // 完全透明
    }

    QPainter painter(&overlayPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制二维码轮廓
    const auto& pos = result.position;

    // 计算视频控件的实际显示区域
    QSize videoSize = m_videoWidget->size();

    // 获取当前摄像头图像的尺寸
    QSize imageSize = m_lastImageSize;

    // 计算从图像坐标系到视频控件坐标系的缩放比例
    double scaleX = static_cast<double>(videoSize.width()) / imageSize.width();
    double scaleY = static_cast<double>(videoSize.height()) / imageSize.height();

    // 保持宽高比，选择较小的缩放比例
    double scale = qMin(scaleX, scaleY);

    // 计算居中偏移
    int offsetX = (videoSize.width() - imageSize.width() * scale) / 2;
    int offsetY = (videoSize.height() - imageSize.height() * scale) / 2;

    // 转换坐标到视频控件坐标系
    auto transformPoint = [scale, offsetX, offsetY](const ZXing::PointI& p) -> QPoint
    { return QPoint(p.x * scale + offsetX, p.y * scale + offsetY); };

    // 绘制轮廓边框
    QPen pen(Qt::red, 3);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // 绘制四边形轮廓
    QPolygon polygon;
    polygon << transformPoint(pos.topLeft()) << transformPoint(pos.topRight())
            << transformPoint(pos.bottomRight()) << transformPoint(pos.bottomLeft());

    painter.drawPolygon(polygon);

    // 绘制角点
    painter.setBrush(Qt::red);
    int pointSize = 8;
    for (const auto& point : {pos.topLeft(), pos.topRight(), pos.bottomRight(), pos.bottomLeft()})
    {
        QPoint transformedPoint = transformPoint(point);
        painter.drawEllipse(transformedPoint, pointSize / 2, pointSize / 2);
    }

    // 绘制识别信息背景和文本
    if (!result.text.isEmpty())
    {
        QFont font = painter.font();
        font.setPointSize(12);
        font.setBold(true);
        painter.setFont(font);

        QFontMetrics fm(font);
        QString displayText =
            result.text.length() > 30 ? result.text.left(27) + "..." : result.text;
        QRect textRect = fm.boundingRect(displayText);
        textRect.adjust(-8, -4, 8, 4);

        // 将文本放在轮廓上方（使用转换后的坐标）
        QPoint topLeftTransformed = transformPoint(pos.topLeft());
        textRect.moveTopLeft(
            QPoint(topLeftTransformed.x(), topLeftTransformed.y() - textRect.height() - 10));

        // 确保文本不超出视频边界
        if (textRect.left() < 0)
            textRect.moveLeft(5);
        if (textRect.right() > videoSize.width())
            textRect.moveRight(videoSize.width() - 5);
        if (textRect.top() < 0)
        {
            QPoint bottomLeftTransformed = transformPoint(pos.bottomLeft());
            textRect.moveTop(bottomLeftTransformed.y() + 10);
        }

        // 绘制半透明背景
        painter.setBrush(QColor(0, 0, 0, 180));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(textRect, 4, 4);

        // 绘制文本
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, displayText);
    }

    // 设置叠加层图像并显示
    m_overlayLabel->setPixmap(overlayPixmap);

    // 确保叠加层位置与视频控件全局位置一致
    QPoint videoGlobalPos = m_videoWidget->mapToGlobal(QPoint(0, 0));
    m_overlayLabel->move(videoGlobalPos);
    m_overlayLabel->resize(m_videoWidget->size());

    m_overlayLabel->show();

    qDebug() << "Overlay settings - Size:" << m_overlayLabel->size()
             << "Video size:" << m_videoWidget->size() << "Pixmap size:" << overlayPixmap.size()
             << "Has alpha:" << overlayPixmap.hasAlphaChannel();

    // 智能叠加层超时：持续到下一次识别成功或超时
    // 停止之前的计时器
    m_overlayTimer->stop();

    // 计算超时时间：识别间隔的2倍 + 1秒，确保持续到下一次识别
    int recognitionInterval = m_recognitionIntervalSlider->value() * 100; // 毫秒
    int overlayTimeout = recognitionInterval * 2 + 1000;                  // 识别间隔的2倍 + 1秒

    // 设置新的超时
    m_overlayTimer->start(overlayTimeout);

    qDebug() << "Drew detection overlay for result:" << result.text
             << "Overlay timeout set to:" << overlayTimeout << "ms";
}

void CameraWidget::applyDefaultStyles()
{
    BaseWidget::applyDefaultStyles();

    // 检测系统主题
    QPalette palette = QApplication::palette();
    bool isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    QString borderColor = isDarkTheme ? "#555555" : "#cccccc";
    
    // 应用特定样式
    setStyleSheet(styleSheet() + QString("QVideoWidget { border: 2px solid %1; border-radius: 8px; }"
                                        "QTextEdit { border: 1px solid %1; border-radius: 4px; }")
                                        .arg(borderColor));
}

void CameraWidget::resizeEvent(QResizeEvent* event)
{
    BaseWidget::resizeEvent(event);

    // 确保叠加层与视频控件大小一致并位置正确
    if (m_overlayLabel && m_videoWidget)
    {
        // 获取视频控件的全局位置
        QPoint videoGlobalPos = m_videoWidget->mapToGlobal(QPoint(0, 0));

        // 设置叠加层全局位置与视频控件一致
        m_overlayLabel->move(videoGlobalPos);
        m_overlayLabel->resize(m_videoWidget->size());

        qDebug() << "Resize event - Overlay repositioned to video position:" << videoGlobalPos
                 << "Size:" << m_overlayLabel->size() << "Video size:" << m_videoWidget->size();
    }
}