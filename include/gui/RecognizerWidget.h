#pragma once

#include "gui/BaseWidget.h"
#include "core/QRCodeRecognizer.h"
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

/**
 * @class RecognizerWidget
 * @brief 二维码识别界面组件
 */
class RecognizerWidget : public BaseWidget
{
    Q_OBJECT

public:
    explicit RecognizerWidget(QWidget* parent = nullptr);
    ~RecognizerWidget() = default;

    /**
     * @brief 设置识别配置
     */
    void setConfig(const QRCodeRecognizer::RecognitionConfig& config);

    /**
     * @brief 获取当前识别配置
     */
    QRCodeRecognizer::RecognitionConfig getConfig() const;

public slots:
    void showRecognitionResult(const QRCodeRecognizer::RecognitionResult& result);
    void showMultiFormatResults(const QList<QRCodeRecognizer::RecognitionResult>& results);
    void showError(const QString& error);

private slots:
    void onLoadFromUrlClicked();
    void onUrlInputChanged();
    void onNetworkImageDownloaded();
    void onNetworkError(QNetworkReply::NetworkError error);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
    void recognizeRequested(const QImage& image);
    void multiFormatRecognizeRequested(const QImage& image);
    void configChanged(const QRCodeRecognizer::RecognitionConfig& config);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onSelectImageClicked();
    void onRecognizeClicked();
    void onMultiFormatClicked();  // 多格式识别槽函数
    void onClearResultsClicked();
    void onSaveResultsClicked();
    void onConfigChanged();
    void onRecognitionCompleted(const QRCodeRecognizer::RecognitionResult& result, int requestId);
    void onRecognitionFailed(const QString& error, int requestId);

private:
    void setupUI();
    void updateImagePreview(const QImage& image);
    void updateResultDisplay(const QRCodeRecognizer::RecognitionResult& result);
    void loadImageFromFile(const QString& filePath);
    void processImageList(const QStringList& filePaths);
    void applyDefaultStyles() override;

private:
    // Core components
    QRCodeRecognizer* m_recognizer;
    
    // UI components - Image area
    QLabel* m_imageLabel;
    QScrollArea* m_imageScrollArea;
    QPushButton* m_selectImageButton;
    QPushButton* m_recognizeButton;
    QPushButton* m_multiFormatButton;  // 多格式识别按钮
    
    // UI components - Network input
    QLineEdit* m_urlLineEdit;
    QPushButton* m_loadFromUrlButton;
    QLabel* m_urlStatusLabel;
    
    // UI components - Configuration
    QGroupBox* m_configGroup;
    QCheckBox* m_tryHarderCheckBox;
    QCheckBox* m_tryRotateCheckBox;
    QCheckBox* m_fastModeCheckBox;
    QSpinBox* m_maxSymbolsSpinBox;
    
    // UI components - Results
    QGroupBox* m_resultsGroup;
    QTextEdit* m_resultsTextEdit;
    QPushButton* m_clearResultsButton;
    QPushButton* m_saveResultsButton;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // Data
    QImage m_currentImage;
    QPixmap m_currentPixmap;
    QList<QRCodeRecognizer::RecognitionResult> m_results;
    int m_requestCounter;
    
    // Network components
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
    QTimer* m_urlValidationTimer;
};
