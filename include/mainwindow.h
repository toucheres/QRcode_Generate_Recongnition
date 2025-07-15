#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

// ZXing includes
#include "WriteBarcode.h"
#include "BarcodeFormat.h"
#include "BitMatrix.h"
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

  private slots:
    void onGenerateQRCode();

  private:
    void setupUI();
    QPixmap generateQRCodePixmap(const QString& text);
    QPixmap zxingMatrixToQPixmap(const ZXing::BitMatrix& matrix);

    QLabel* m_titleLabel;
    QWidget* m_centralWidget;
    QLineEdit* m_textInput;
    QPushButton* m_generateButton;
    QLabel* m_qrCodeLabel;
};
