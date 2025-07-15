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
    
    // 通用
    void onModeChanged(int index);

  private:
    void setupUI();
    void setupGenerateMode();
    void setupRecognizeMode();
    
    // 生成相关
    QPixmap generateQRCodePixmap(const QString& text);
    QPixmap zxingMatrixToQPixmap(const ZXing::BitMatrix& matrix);
    
    // 识别相关
    QString recognizeQRCodeFromImage(const QImage& image);
    QString recognizeQRCodeFromPixmap(const QPixmap& pixmap);
    void displayRecognitionResult(const QString& result);
    void displaySelectedImage(const QPixmap& pixmap);

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
    
    // 网络相关
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
};
