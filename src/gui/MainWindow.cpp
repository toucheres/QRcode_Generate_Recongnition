#include "gui/MainWindow.h"
#include "gui/GeneratorWidget.h"
#include "gui/RecognizerWidget.h"
#include "gui/CameraWidget.h"
#include "core/QRCodeGenerator.h"
#include "core/QRCodeRecognizer.h"
#include "utils/AppUtils.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QSplitter>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_generatorWidget(nullptr)
    , m_recognizerWidget(nullptr)
    , m_cameraWidget(nullptr)
    , m_generator(new QRCodeGenerator())
    , m_recognizer(new QRCodeRecognizer(this))
    , m_hasUnsavedChanges(false)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    setupConnections();
    loadSettings();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete m_generator;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_hasUnsavedChanges) {
        int ret = QMessageBox::question(
            this,
            "确认退出",
            "有未保存的更改，确定要退出吗？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
        
        if (ret == QMessageBox::Save) {
            onSaveProject();
            event->accept();
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::onNewProject()
{
    if (m_hasUnsavedChanges) {
        int ret = QMessageBox::question(
            this,
            "新建项目",
            "当前有未保存的更改，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        
        if (ret == QMessageBox::Save) {
            onSaveProject();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }
    
    // 重置界面状态
    if (m_generatorWidget) {
        QRCodeGenerator::GenerationConfig defaultConfig;
        m_generatorWidget->setConfig(defaultConfig);
    }
    
    m_currentProjectFile.clear();
    m_hasUnsavedChanges = false;
    updateWindowTitle();
    statusBar()->showMessage("新建项目", 2000);
}

void MainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开图片文件",
        "",
        AppUtils::getImageFileFilter()
    );
    
    if (!fileName.isEmpty()) {
        // 这里可以实现打开图片进行识别的功能
        statusBar()->showMessage(QString("打开文件: %1").arg(fileName), 2000);
    }
}

void MainWindow::onSaveProject()
{
    QString fileName = m_currentProjectFile;
    
    if (fileName.isEmpty()) {
        fileName = QFileDialog::getSaveFileName(
            this,
            "保存项目",
            "",
            "项目文件 (*.qrproj);;所有文件 (*.*)"
        );
    }
    
    if (!fileName.isEmpty()) {
        // 这里可以实现保存项目设置的功能
        m_currentProjectFile = fileName;
        m_hasUnsavedChanges = false;
        updateWindowTitle();
        statusBar()->showMessage("项目已保存", 2000);
    }
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onAbout()
{
    QMessageBox::about(
        this,
        "关于",
        "<h3>QR码生成识别器 v2.0</h3>"
        "<p>一个基于Qt6和ZXing-C++的二维码处理工具</p>"
        "<p><b>功能特性:</b></p>"
        "<ul>"
        "<li>支持多种二维码格式生成</li>"
        "<li>支持Logo嵌入</li>"
        "<li>支持二维码识别</li>"
        "<li>支持中文等Unicode字符</li>"
        "</ul>"
        "<p><b>技术栈:</b> Qt6 + ZXing-C++ + CMake</p>"
        "<p><b>开发者:</b> SCU-CS</p>"
    );
}

void MainWindow::onSettings()
{
    // 这里可以实现设置对话框
    QMessageBox::information(this, "设置", "设置功能将在后续版本中实现");
}

void MainWindow::onGenerateQRCode()
{
    // 从生成器控件获取配置并生成二维码
    if (m_generatorWidget) {
        auto config = m_generatorWidget->getConfig();
        QPixmap qrCode = m_generator->generateQRCode(config);
        
        if (!qrCode.isNull()) {
            m_generatorWidget->showGeneratedQRCode(qrCode);
            statusBar()->showMessage("二维码生成成功", 2000);
            m_hasUnsavedChanges = true;
        } else {
            m_generatorWidget->showError(m_generator->getLastError());
            statusBar()->showMessage("二维码生成失败", 2000);
        }
    }
}

void MainWindow::onSaveQRCode(const QPixmap& pixmap)
{
    if (saveQRCodeToFile(pixmap)) {
        statusBar()->showMessage("二维码图片保存成功", 2000);
    }
}

void MainWindow::onRecognizeQRCode(const QImage& image)
{
    // 这里可以实现图片识别的处理逻辑
    // 例如显示进度、结果等
    statusBar()->showMessage("正在识别二维码...", 1000);
}

void MainWindow::onRecognizeMultiFormat(const QImage& image)
{
    if (image.isNull()) {
        statusBar()->showMessage("图像无效", 3000);
        return;
    }
    
    statusBar()->showMessage("正在识别多种条码格式...", 2000);
    
    // 使用多格式识别
    QRCodeRecognizer::RecognitionConfig config = m_recognizerWidget->getConfig();
    auto results = m_recognizer->recognizeMultiFormat(image, config);
    
    if (results.isEmpty()) {
        m_recognizerWidget->showError("未识别到任何条码格式");
        statusBar()->showMessage("识别失败", 3000);
    } else {
        m_recognizerWidget->showMultiFormatResults(results);
        QString message = QString("识别成功 - 找到%1种格式").arg(results.size());
        statusBar()->showMessage(message, 5000);
    }
}

void MainWindow::onQRCodeDetected(const QString& text)
{
    QString message = QString("检测到二维码: %1").arg(text.left(50));
    if (text.length() > 50) {
        message += "...";
    }
    statusBar()->showMessage(message, 3000);
}

void MainWindow::onCameraError(const QString& error)
{
    statusBar()->showMessage(QString("摄像头错误: %1").arg(error), 5000);
    QMessageBox::warning(this, "摄像头错误", error);
}

void MainWindow::setupUI()
{
    setMinimumSize(900, 700);
    resize(1200, 800);
    
    // 创建中央控件
    QWidget* centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabPosition(QTabWidget::North);
    m_tabWidget->setMovable(true);
    
    // 创建生成器页面
    m_generatorWidget = new GeneratorWidget();
    m_tabWidget->addTab(m_generatorWidget, "二维码生成");
    
    // 创建识别器页面
    m_recognizerWidget = new RecognizerWidget();
    m_tabWidget->addTab(m_recognizerWidget, "二维码识别");
    
    // 创建摄像头页面
    m_cameraWidget = new CameraWidget();
    m_tabWidget->addTab(m_cameraWidget, "摄像头识别");
    
    mainLayout->addWidget(m_tabWidget);
    
    // 设置样式
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #f0f0f0;"
        "}"
        "QTabWidget::pane {"
        "    border: 1px solid #cccccc;"
        "    background-color: white;"
        "}"
        "QTabBar::tab {"
        "    background-color: #e0e0e0;"
        "    padding: 8px 16px;"
        "    margin-right: 2px;"
        "    border-top-left-radius: 4px;"
        "    border-top-right-radius: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: white;"
        "    border-bottom: 1px solid white;"
        "}"
        "QTabBar::tab:hover {"
        "    background-color: #f0f0f0;"
        "}"
    );
}

void MainWindow::setupMenuBar()
{
    m_menuBar = menuBar();
    
    // 文件菜单
    QMenu* fileMenu = m_menuBar->addMenu("文件(&F)");
    
    QAction* newAction = fileMenu->addAction("新建项目(&N)");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewProject);
    
    QAction* openAction = fileMenu->addAction("打开文件(&O)");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    
    fileMenu->addSeparator();
    
    QAction* saveAction = fileMenu->addAction("保存项目(&S)");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveProject);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    
    // 工具菜单
    QMenu* toolsMenu = m_menuBar->addMenu("工具(&T)");
    
    QAction* settingsAction = toolsMenu->addAction("设置(&S)");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);
    
    // 帮助菜单
    QMenu* helpMenu = m_menuBar->addMenu("帮助(&H)");
    
    QAction* aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    
    QAction* aboutQtAction = helpMenu->addAction("关于Qt(&Q)");
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage("准备就绪");
    
    // 添加永久状态信息
    QLabel* versionLabel = new QLabel("v2.0");
    versionLabel->setStyleSheet("color: #666666; margin-right: 10px;");
    m_statusBar->addPermanentWidget(versionLabel);
}

void MainWindow::setupConnections()
{
    // 连接生成器信号
    if (m_generatorWidget) {
        connect(m_generatorWidget, &GeneratorWidget::generateRequested,
                this, &MainWindow::onGenerateQRCode);
        connect(m_generatorWidget, &GeneratorWidget::saveRequested,
                this, &MainWindow::onSaveQRCode);
    }
    
    // 连接识别器信号
    if (m_recognizerWidget) {
        connect(m_recognizerWidget, &RecognizerWidget::recognizeRequested,
                this, &MainWindow::onRecognizeQRCode);
        connect(m_recognizerWidget, &RecognizerWidget::multiFormatRecognizeRequested,
                this, &MainWindow::onRecognizeMultiFormat);
    }
    
    // 连接摄像头信号
    if (m_cameraWidget) {
        connect(m_cameraWidget, &CameraWidget::qrCodeDetected,
                this, [this](const QString& text, const auto& result) {
                    Q_UNUSED(result)
                    onQRCodeDetected(text);
                });
        connect(m_cameraWidget, &CameraWidget::cameraError,
                this, &MainWindow::onCameraError);
    }
}

void MainWindow::updateWindowTitle(const QString& subtitle)
{
    QString title = "QR码生成识别器";
    
    if (!subtitle.isEmpty()) {
        title += " - " + subtitle;
    } else if (!m_currentProjectFile.isEmpty()) {
        QFileInfo fileInfo(m_currentProjectFile);
        title += " - " + fileInfo.baseName();
    }
    
    if (m_hasUnsavedChanges) {
        title += " *";
    }
    
    setWindowTitle(title);
}

bool MainWindow::saveQRCodeToFile(const QPixmap& pixmap)
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存二维码图片",
        "",
        AppUtils::getSaveImageFileFilter()
    );
    
    if (fileName.isEmpty()) {
        return false;
    }
    
    bool success = false;
    QString extension = AppUtils::getFileExtension(fileName);
    
    if (extension == "svg") {
        // SVG格式需要特殊处理
        QMessageBox::information(this, "提示", "SVG格式保存功能将在后续版本中完善");
        success = false;
    } else {
        success = pixmap.save(fileName);
    }
    
    if (success) {
        QMessageBox::information(this, "保存成功", 
                                QString("二维码图片已保存到:\n%1").arg(fileName));
    } else {
        QMessageBox::warning(this, "保存失败", "保存二维码图片失败！");
    }
    
    return success;
}

void MainWindow::loadSettings()
{
    QSettings settings;
    
    // 恢复窗口几何尺寸
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    // 恢复标签页状态
    int currentTab = settings.value("currentTab", 0).toInt();
    if (currentTab < m_tabWidget->count()) {
        m_tabWidget->setCurrentIndex(currentTab);
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    
    // 保存窗口几何尺寸
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    
    // 保存标签页状态
    settings.setValue("currentTab", m_tabWidget->currentIndex());
}
