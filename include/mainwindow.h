#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

  private slots:
    void onButtonClicked();
    void onGenerateQRCode();

  private:
    void setupUI();
    QPixmap generateQRCodePixmap(const QString& text);

    QLabel* m_titleLabel;
    QPushButton* m_pushButton;
    QWidget* m_centralWidget;
    QLineEdit* m_textInput;
    QPushButton* m_generateButton;
    QLabel* m_qrCodeLabel;
};
