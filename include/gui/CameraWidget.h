#pragma once

#include "gui/BaseWidget.h"
#include "core/QRCodeRecognizer.h"
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QImageCapture>
#include <QVideoFrame>
#include <QVideoSink>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QTimer>
#include <QGroupBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>

/**
 * @class CameraWidget
 * @brief 摄像头实时二维码识别界面组件
 */
class CameraWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget* parent = nullptr);
    ~CameraWidget();

    /**
     * @brief 开始摄像头预览
     */
    void startCamera();

    /**
     * @brief 停止摄像头预览
     */
    void stopCamera();

    /**
     * @brief 检查摄像头是否可用
     */
    bool isCameraAvailable() const;

public slots:
    void onRecognitionResult(const QRCodeRecognizer::RecognitionResult& result);

signals:
    void qrCodeDetected(const QString& text, const QRCodeRecognizer::RecognitionResult& result);
    void cameraError(const QString& error);

private slots:
    void onCameraToggleClicked();
    void onCaptureClicked();
    void onClearHistoryClicked();
    void onSaveHistoryClicked();
    void onCameraDeviceChanged();
    void onResolutionChanged();
    void onRecognitionSettingsChanged();
    void onVideoFrameChanged(const QVideoFrame& frame);
    void onRecognitionTimer();
    void onCameraErrorOccurred();

private:
    void setupUI();
    void setupCamera();
    void updateCameraDevices();
    void updateResolutions();
    void processVideoFrame(const QVideoFrame& frame);
    void processVideoImage(const QImage& image);
    void addResultToHistory(const QRCodeRecognizer::RecognitionResult& result);
    void drawDetectionOverlay(const QRCodeRecognizer::RecognitionResult& result);
    void applyDefaultStyles() override;

private:
    // Core components
    QRCodeRecognizer* m_recognizer;
    
    // Camera components
    QCamera* m_camera;
    QMediaCaptureSession* m_captureSession;
    QVideoWidget* m_videoWidget;
    QImageCapture* m_imageCapture;
    QVideoSink* m_videoSink;
    
    // UI components - Camera controls
    QGroupBox* m_cameraGroup;
    QComboBox* m_deviceCombo;
    QComboBox* m_resolutionCombo;
    QPushButton* m_cameraToggleButton;
    QPushButton* m_captureButton;
    QLabel* m_cameraStatusLabel;
    
    // UI components - Recognition settings
    QGroupBox* m_recognitionGroup;
    QCheckBox* m_realtimeRecognitionCheckBox;
    QCheckBox* m_showOverlayCheckBox;
    QSlider* m_recognitionIntervalSlider;
    QLabel* m_intervalLabel;
    QCheckBox* m_autoSaveCheckBox;
    
    // UI components - Results
    QGroupBox* m_resultsGroup;
    QTextEdit* m_historyTextEdit;
    QPushButton* m_clearHistoryButton;
    QPushButton* m_saveHistoryButton;
    QLabel* m_detectionCountLabel;
    
    // Recognition timer and settings
    QTimer* m_recognitionTimer;
    QVideoFrame m_currentFrame;
    QList<QRCodeRecognizer::RecognitionResult> m_detectionHistory;
    
    // State
    bool m_cameraActive;
    bool m_realtimeRecognition;
    int m_detectionCount;
    QDateTime m_lastDetectionTime;
    QString m_lastDetectedText;
};
