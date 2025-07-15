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

// Qt Multimedia includes
#include <QCamera>
#include <QVideoWidget>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QVideoFrame>

// ZXing includes
#include "WriteBarcode.h"
#include "BarcodeFormat.h"
#include "BitMatrix.h"
#include "ReadBarcode.h"
#include "ImageView.h"

#ifndef ZXING_EXPERIMENTAL_API
#include "CharacterSet.h"
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

  private slots:
    // 生成模式
    void onGenerateQRCode();
    
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
    QPixmap generateQRCodePixmap(const QString& text);
    QPixmap zxingMatrixToQPixmap(const ZXing::BitMatrix& matrix);
    
    // 识别相关
    QString recognizeQRCodeFromImage(const QImage& image);
    QString recognizeQRCodeFromPixmap(const QPixmap& pixmap);
    void displayRecognitionResult(const QString& result);
    void displaySelectedImage(const QPixmap& pixmap);
    
    // 摄像头相关
    void recognizeFromVideoFrame();
    void checkCameraPermissions();  // 新增
    void debugCameraInfo();         // 新增

    // UI组件 - 主界面
    QWidget* m_centralWidget;
    QTabWidget* m_tabWidget;
    
    // UI组件 - 生成模式
    QWidget* m_generateWidget;
    QLabel* m_generateTitleLabel;
    QLineEdit* m_textInput;
    QPushButton* m_generateButton;
    QLabel* m_qrCodeLabel;
    
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
