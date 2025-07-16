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
    
    // 摄像头模式
    void onToggleCamera();
    void onCaptureImage();
    void onCameraChanged(int index);
    void onImageCaptured(int id, const QImage& image);
    void onCameraError(QCamera::Error error);
    void onRecognitionTimerTimeout();
    
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
    
    // 识别相关
    QString recognizeQRCodeFromImage(const QImage& image);
    QString recognizeQRCodeFromPixmap(const QPixmap& pixmap);
    void displayRecognitionResult(const QString& result);
    void displaySelectedImage(const QPixmap& pixmap);
    
    // 摄像头相关
    void recognizeFromVideoFrame();
    void checkCameraPermissions();
    void debugCameraInfo();

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
    
    // UI组件 - 摄像头识别
    QWidget* m_cameraWidget;
    QComboBox* m_cameraComboBox;
    QPushButton* m_toggleCameraButton;
    QPushButton* m_captureButton;
    QVideoWidget* m_videoWidget;
    QLabel* m_cameraStatusLabel;
    QTextEdit* m_cameraResultTextEdit;
    
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
};