#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

// Qt Multimedia includes
#include <QCamera>
#include <QVideoWidget>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QVideoFrame>

// ZXing includes - 确保正确的头文件顺序和包含
#include "BarcodeFormat.h"
#include "BitMatrix.h"
#include "ReadBarcode.h"
#include "ImageView.h"

// 检查实验性API是否可用
#ifdef ZXING_EXPERIMENTAL_API
    #include "WriteBarcode.h"
    #define USE_EXPERIMENTAL_API 1
#else
    #include "MultiFormatWriter.h"
    #include "CharacterSet.h"
    #define USE_EXPERIMENTAL_API 0
#endif

// 新增：识别工作线程类
class QRRecognitionWorker : public QObject
{
    Q_OBJECT

public:
    explicit QRRecognitionWorker(QObject* parent = nullptr);
    ~QRRecognitionWorker();

public slots:
    void processImage(const QImage& image, int requestId);
    void stop();

signals:
    void recognitionCompleted(const QString& result, int requestId);
    void recognitionFailed(const QString& error, int requestId);

private:
    QString recognizeQRCodeFromImage(const QImage& image);
    bool m_shouldStop;
    QMutex m_mutex;
};

// 新增：图像队列管理器
class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject* parent = nullptr);
    ~ImageProcessor();

    void addImage(const QImage& image);
    void start();
    void stop();
    void setProcessingInterval(int ms);

signals:
    void imageReady(const QImage& image, int requestId);
    void recognitionResult(const QString& result, int requestId);

public slots:
    void onRecognitionCompleted(const QString& result, int requestId);
    void onRecognitionFailed(const QString& error, int requestId);

private slots:
    void processNextImage();

private:
    struct ImageRequest {
        QImage image;
        int requestId;
        qint64 timestamp;
    };

    QQueue<ImageRequest> m_imageQueue;
    QTimer* m_processTimer;
    QThread* m_workerThread;
    QRRecognitionWorker* m_worker;
    QMutex m_queueMutex;
    int m_currentRequestId;
    int m_lastProcessedRequestId;
    static const int MAX_QUEUE_SIZE = 3;  // 限制队列大小避免内存堆积
    static const int PROCESSING_INTERVAL = 1000;  // 默认1秒处理一次
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

  private slots:
    // 生成模式
    void onSaveQRCode(); // 新增：保存二维码图片槽函数
    void onGenerateQRCode();
    void onSelectLogoImage();  // 新增：选择中心图片
    void onEmbedLogoChanged(bool enabled);  // 新增：启用/禁用图片嵌入
    void onLogoSizeChanged(int size);  // 新增：调整图片大小
  
    // 识别模式
    void onSelectImageFile();
    void onLoadFromUrl();
    void onUrlInputChanged();
    void onNetworkReplyFinished();
    void onCopyRecognizedText();     // 新增：复制识别结果
    void onOpenRecognizedUrl();      // 新增：打开识别的网址
    
    // 摄像头模式
    void onToggleCamera();
    void onCaptureImage();
    void onCameraChanged(int index);
    void onImageCaptured(int id, const QImage& image);
    void onCameraError(QCamera::Error error);
    void onRecognitionTimerTimeout();
    void onClearHistory();  // 新增：清空历史记录
    void onAutoOpenUrlChanged(bool enabled);  // 新增：自动跳转网址选项变化
    void onAutoCopyChanged(bool enabled);     // 新增：自动复制选项变化
    
    // 新增：异步识别结果处理
    void onAsyncRecognitionResult(const QString& result, int requestId);
    
    // 通用
    void onModeChanged(int index);

  private:
    void setupUI();
    void setupGenerateMode();
    void setupRecognizeMode();
    void setupCameraRecognition();
    void initializeCamera();
    void startCamera();
    void stopCamera();
    void updateCameraList();
    int index_ =0 ;
    // 生成相关
    QLabel* m_qrCodeLabel;//新增：保存槽函数
    QPushButton* m_saveQRCodeButton; // 新增：保存二维码按钮
    QPixmap generateQRCodePixmap(const QString& text);
    QPixmap zxingMatrixToQPixmap(const ZXing::BitMatrix& matrix);
    QPixmap embedLogoInQRCode(const QPixmap& qrCode, const QPixmap& logo, int logoSizePercent = 20);  // 新增
    
    // 新增：错误处理方法
    QPixmap generateQRCodeWithFallback(const QString& text, const QString& ecLevel, int size);
    QPixmap generateMinimalQRCode(const QString& text);  // 最简化生成方法
    QPixmap createErrorQRCode();  // 创建错误提示图像
    
    // 识别相关 - 修改为同步版本（用于单张图片识别）
    QString recognizeQRCodeFromImage(const QImage& image);
    QString recognizeQRCodeFromPixmap(const QPixmap& pixmap);
    void displayRecognitionResult(const QString& result);
    void displaySelectedImage(const QPixmap& pixmap);
    void updateRecognizeActionButtons(const QString& result);  // 新增：更新操作按钮状态
    
    // 摄像头相关 - 修改为异步处理
    void startAsyncRecognition(const QImage& image);  // 新增：启动异步识别
    void checkCameraPermissions();
    void debugCameraInfo();
    void addToHistory(const QString& result);  // 新增：添加到历史记录
    void updateHistoryDisplay();  // 新增：更新历史记录显示
    void showRecognitionHint(const QString& result);  // 新增：显示识别提示
    void handleAutoActions(const QString& result);  // 新增：处理自动操作
    bool isValidUrl(const QString& text);  // 新增：判断是否为有效网址
    void autoOpenUrl(const QString& url);  // 新增：自动打开网址
    void autoCopyToClipboard(const QString& text);  // 新增：自动复制到剪贴板
    void recognizeFromVideoFrame();
    // UI组件 - 主界面
    QWidget* m_centralWidget;
    QTabWidget* m_tabWidget;
    
    // UI组件 - 生成模式
    QWidget* m_generateWidget;
    QLabel* m_generateTitleLabel;
    QLineEdit* m_textInput;
    QPushButton* m_generateButton;
    
    
    // 新增：二维码生成选项控件
    QGroupBox* m_qrOptionsGroup;
    QComboBox* m_errorCorrectionCombo;
    QSpinBox* m_qrSizeSpinBox;
    QCheckBox* m_embedLogoCheckBox;
    QPushButton* m_selectLogoButton;
    QLabel* m_logoPreviewLabel;
    QSlider* m_logoSizeSlider;
    QLabel* m_logoSizeLabel;
    QComboBox* m_qrformat;
    // 新增：存储logo图片
    QPixmap m_logoPixmap;
    
    // UI组件 - 识别模式
    QWidget* m_recognizeWidget;
    QLabel* m_recognizeTitleLabel;
    QPushButton* m_selectFileButton;
    QLineEdit* m_urlInput;
    QPushButton* m_loadUrlButton;
    QLabel* m_imageDisplayLabel;
    QTextEdit* m_resultTextEdit;
    QLabel* m_statusLabel;
    
    // 新增：识别模式操作按钮
    QGroupBox* m_recognizeActionsGroup;
    QPushButton* m_copyResultButton;
    QPushButton* m_openUrlButton;
    QString m_currentRecognizedText;  // 存储当前识别结果
    
    // UI组件 - 摄像头识别
    QWidget* m_cameraWidget;
    QComboBox* m_cameraComboBox;
    QPushButton* m_toggleCameraButton;
    QPushButton* m_captureButton;
    QVideoWidget* m_videoWidget;
    QLabel* m_cameraStatusLabel;
    QTextEdit* m_cameraResultTextEdit;
    
    // 新增：历史记录相关UI
    QGroupBox* m_historyGroup;
    QTextEdit* m_historyTextEdit;
    QPushButton* m_clearHistoryButton;
    QLabel* m_recognitionHintLabel;
    
    // 新增：自动功能相关UI
    QGroupBox* m_autoActionsGroup;
    QCheckBox* m_autoOpenUrlCheckBox;
    QCheckBox* m_autoCopyCheckBox;
    
    // 网络相关
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
    
    // 摄像头相关
    QCamera* m_camera;
    QImageCapture* m_imageCapture;
    QMediaCaptureSession* m_captureSession;
    QTimer* m_recognitionTimer;
    bool m_cameraActive;
    QList<QCameraDevice> m_availableCameras;
    
    // 新增：历史记录相关
    QStringList m_recognitionHistory;
    QString m_lastRecognizedContent;
    QTimer* m_hintTimer;  // 用于控制提示显示时间
    
    // 新增：自动功能相关
    bool m_autoOpenUrlEnabled;
    bool m_autoCopyEnabled;
    
    // 新增：多线程相关成员
    ImageProcessor* m_imageProcessor;
    int m_lastCaptureRequestId;  // 跟踪最后的拍照请求ID
    QMutex m_recognitionMutex;   // 保护识别结果的互斥锁
    
    // 新增：性能监控
    qint64 m_lastRecognitionTime;  // 上次识别时间
    int m_recognitionCount;        // 识别计数
};