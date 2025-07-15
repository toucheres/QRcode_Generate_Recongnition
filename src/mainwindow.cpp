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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_titleLabel(nullptr)
    , m_pushButton(nullptr)
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
    m_textInput->setPlaceholderText("请输入要生成二维码的文本...");
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
    
    // 创建原来的测试按钮
    m_pushButton = new QPushButton("测试按钮", this);
    m_pushButton->setStyleSheet("QPushButton { font-size: 14px; padding: 10px 20px; }");
    
    // 连接信号和槽
    connect(m_pushButton, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateQRCode);
    connect(m_textInput, &QLineEdit::returnPressed, this, &MainWindow::onGenerateQRCode);
    
    // 添加到布局
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_textInput);
    mainLayout->addWidget(m_generateButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_qrCodeLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_pushButton, 0, Qt::AlignCenter);
    mainLayout->addStretch();
}

void MainWindow::onButtonClicked()
{
    QMessageBox::information(this, "提示", "您点击了按钮！");
}

void MainWindow::onGenerateQRCode()
{
    QString text = m_textInput->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要生成二维码的文本！");
        return;
    }
    
    QPixmap qrPixmap = generateQRCodePixmap(text);
    if (!qrPixmap.isNull()) {
        m_qrCodeLabel->setPixmap(qrPixmap);
        m_qrCodeLabel->setText("");
    } else {
        QMessageBox::critical(this, "错误", "生成二维码失败！");
    }
}

QPixmap MainWindow::generateQRCodePixmap(const QString& text)
{
    // 简单的模拟二维码生成 - 实际项目中需要使用QR码库如qrencode
    QPixmap pixmap(300, 300);
    pixmap.fill(Qt::white);
    
    QPainter painter(&pixmap);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);
    
    // 绘制简单的网格模式作为示例
    int gridSize = 10;
    int offset = 50;
    
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            // 基于文本内容生成伪随机图案
            int hash = qHash(text + QString::number(i) + QString::number(j));
            if (hash % 3 == 0) {
                painter.fillRect(offset + i * gridSize, offset + j * gridSize, gridSize, gridSize, Qt::black);
            }
        }
    }
    
    // 绘制定位标记
    painter.fillRect(offset, offset, gridSize * 3, gridSize * 3, Qt::black);
    painter.fillRect(offset + gridSize * 16, offset, gridSize * 3, gridSize * 3, Qt::black);
    painter.fillRect(offset, offset + gridSize * 16, gridSize * 3, gridSize * 3, Qt::black);
    
    return pixmap;
}
