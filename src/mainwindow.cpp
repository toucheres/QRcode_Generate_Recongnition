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

// ZXing includes
#ifdef ZXING_EXPERIMENTAL_API
#include "ZXing/WriteBarcode.h"
#include "ZXing/BarcodeFormat.h"
#else
#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "BarcodeFormat.h"
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_titleLabel(nullptr)
    , m_centralWidget(nullptr)
    , m_textInput(nullptr)
    , m_generateButton(nullptr)
    , m_qrCodeLabel(nullptr)
{
    setupUI();
    setWindowTitle("QR码生成识别器");
    resize(500, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央控件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    
    // 创建标题标签
    m_titleLabel = new QLabel("QR码生成器", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: #333; margin: 20px; }");
    
    // 创建输入框
    m_textInput = new QLineEdit(this);
    m_textInput->setPlaceholderText("请输入要生成二维码的文本（支持中文）...");
    m_textInput->setStyleSheet("QLineEdit { font-size: 14px; padding: 10px; margin: 10px 0; }");
    
    // 创建生成按钮
    m_generateButton = new QPushButton("生成二维码", this);
    m_generateButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; background-color: #007ACC; color: white; border: none; border-radius: 5px; } QPushButton:hover { background-color: #005a9e; }");
    
    // 创建二维码显示标签
    m_qrCodeLabel = new QLabel(this);
    m_qrCodeLabel->setAlignment(Qt::AlignCenter);
    m_qrCodeLabel->setMinimumSize(300, 300);
    m_qrCodeLabel->setStyleSheet("QLabel { border: 2px dashed #ccc; background-color: #f9f9f9; }");
    m_qrCodeLabel->setText("二维码将显示在这里");
    
    // 连接信号和槽
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateQRCode);
    connect(m_textInput, &QLineEdit::returnPressed, this, &MainWindow::onGenerateQRCode);
    
    // 添加到布局
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_textInput);
    mainLayout->addWidget(m_generateButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch();
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