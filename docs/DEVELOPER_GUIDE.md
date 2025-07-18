# 开发者指南

本文档为希望参与QR码生成识别器开发的开发者提供详细指导。

## 🏗️ 项目架构

### 目录结构
```
QRcode_Generate_Recongnition/
├── src/
│   ├── main.cpp                 # 程序入口点
│   ├── mainwindow.cpp/h         # 主窗口控制器
│   └── widgets/                 # UI组件模块
│       ├── BaseWidget.cpp/h     # UI基类（主题检测）
│       ├── GeneratorWidget.cpp/h # 生成器界面
│       ├── RecognizerWidget.cpp/h # 识别器界面
│       ├── CameraWidget.cpp/h   # 摄像头识别
│       └── SettingsDialog.cpp/h # 设置对话框
├── include/
│   ├── mainwindow.h             # 主窗口头文件
│   └── widgets/                 # UI组件头文件
├── core/
│   ├── QRCodeGenerator.cpp/h    # 二维码生成核心
│   └── QRCodeRecognizer.cpp/h   # 二维码识别核心
├── utils/
│   └── AppSettings.cpp/h        # 应用设置管理
├── forms/
│   └── qrc.qrc                  # 资源文件
├── 3rd/
│   └── zxing/                   # ZXing-C++库
├── docs/                        # 文档目录
├── build/                       # 构建输出目录
└── CMakeLists.txt              # CMake配置文件
```

### 核心组件

#### 1. 主窗口架构 (MainWindow)
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    QTabWidget* m_tabWidget;         // 主标签页容器
    GeneratorWidget* m_generator;    // 生成器组件
    RecognizerWidget* m_recognizer;  // 识别器组件
    CameraWidget* m_camera;          // 摄像头组件
    QMenuBar* m_menuBar;            // 菜单栏
    QStatusBar* m_statusBar;        // 状态栏
};
```

#### 2. 基础组件架构 (BaseWidget)
```cpp
class BaseWidget : public QWidget {
    Q_OBJECT

protected:
    bool isDarkTheme() const;        // 检测当前主题
    virtual void setupUI() = 0;      // 纯虚函数，子类实现UI
    virtual void connectSignals() {} // 信号连接（可选实现）
};
```

#### 3. 设置管理系统 (AppSettings)
```cpp
class AppSettings {
public:
    // 单例模式
    static AppSettings& instance();
    
    // URL自动打开设置
    static bool autoOpenUrls();
    static void setAutoOpenUrls(bool enable);
    static bool tryOpenUrl(const QString& url);
    
    // 设置持久化
    static void saveSettings();
    static void loadSettings();
    static void resetToDefaults();

private:
    QSettings m_settings;            // Qt设置存储
    static AppSettings* s_instance;  // 单例实例
};
```

## 🎨 UI开发指南

### 1. 响应式设计原则

#### 使用QScrollArea处理小窗口
```cpp
void GeneratorWidget::setupUI() {
    // 创建滚动区域
    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 创建内容小部件
    auto contentWidget = new QWidget();
    auto contentLayout = new QVBoxLayout(contentWidget);
    
    // 添加所有UI组件到contentLayout
    contentLayout->addWidget(createFormatGroup());
    contentLayout->addWidget(createOptionsGroup());
    contentLayout->addWidget(createPreviewGroup());
    
    // 设置滚动区域
    scrollArea->setWidget(contentWidget);
    
    // 主布局
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea);
}
```

#### 主题适配
```cpp
bool BaseWidget::isDarkTheme() const {
    // 检测系统主题
    QPalette palette = QApplication::palette();
    return palette.color(QPalette::Window).lightness() < 128;
}

void CustomWidget::updateTheme() {
    if (isDarkTheme()) {
        setStyleSheet("QWidget { background-color: #2b2b2b; color: #ffffff; }");
    } else {
        setStyleSheet("QWidget { background-color: #ffffff; color: #000000; }");
    }
}
```

### 2. 自定义组件开发

#### 创建新的Widget组件
```cpp
// 1. 继承BaseWidget
class NewWidget : public BaseWidget {
    Q_OBJECT

public:
    explicit NewWidget(QWidget* parent = nullptr);

protected:
    void setupUI() override;
    void connectSignals() override;

private slots:
    void onActionTriggered();

private:
    QPushButton* m_actionButton;
    QLabel* m_statusLabel;
};

// 2. 实现构造函数
NewWidget::NewWidget(QWidget* parent) : BaseWidget(parent) {
    setupUI();
    connectSignals();
}

// 3. 实现UI设置
void NewWidget::setupUI() {
    auto layout = new QVBoxLayout(this);
    
    m_actionButton = new QPushButton("Action", this);
    m_statusLabel = new QLabel("Ready", this);
    
    layout->addWidget(m_actionButton);
    layout->addWidget(m_statusLabel);
}

// 4. 实现信号连接
void NewWidget::connectSignals() {
    connect(m_actionButton, &QPushButton::clicked, this, &NewWidget::onActionTriggered);
}
```

## 🔧 核心功能开发

### 1. 二维码生成器扩展

#### 添加新的条码格式
```cpp
// 在BarcodeFormat枚举中添加新格式
enum class BarcodeFormat {
    QRCode,
    PDF417,
    Code128,
    NewFormat,  // 新增格式
    // ...
};

// 在QRCodeGenerator中实现格式特定逻辑
QString QRCodeGenerator::prepareTextForFormat(const QString& text, BarcodeFormat format) {
    switch (format) {
        case BarcodeFormat::NewFormat:
            // 实现新格式的文本预处理
            return processNewFormat(text);
        // ...
    }
}

bool QRCodeGenerator::validateFormatAndText(BarcodeFormat format, const QString& text) {
    switch (format) {
        case BarcodeFormat::NewFormat:
            // 实现新格式的验证逻辑
            return text.length() <= MAX_NEW_FORMAT_LENGTH;
        // ...
    }
}
```

#### 添加生成选项
```cpp
struct GenerationOptions {
    BarcodeFormat format = BarcodeFormat::QRCode;
    int size = 300;
    int errorCorrection = 1;  // L=0, M=1, Q=2, H=3
    bool hasLogo = false;
    QString logoPath;
    int logoSize = 50;
    QColor foregroundColor = Qt::black;    // 新增：前景色
    QColor backgroundColor = Qt::white;    // 新增：背景色
    int margin = 10;                       // 新增：边距
};
```

### 2. 识别器功能扩展

#### 添加新的识别源
```cpp
class NewRecognitionSource {
public:
    virtual QList<QRCodeResult> recognize() = 0;
    virtual bool isAvailable() const = 0;
};

class NetworkImageRecognizer : public NewRecognitionSource {
public:
    NetworkImageRecognizer(const QString& url);
    QList<QRCodeResult> recognize() override;
    bool isAvailable() const override;
    
private:
    QString m_url;
    QNetworkAccessManager m_network;
};
```

#### 结果处理扩展
```cpp
class ResultProcessor {
public:
    static void processResult(const QString& result, QWidget* parent) {
        if (isUrl(result)) {
            handleUrl(result, parent);
        } else if (isEmail(result)) {
            handleEmail(result, parent);
        } else if (isPhoneNumber(result)) {
            handlePhoneNumber(result, parent);
        } else if (isWiFiConfig(result)) {
            handleWiFiConfig(result, parent);
        }
        // 添加更多类型处理
    }

private:
    static bool isEmail(const QString& text);
    static bool isPhoneNumber(const QString& text);
    static bool isWiFiConfig(const QString& text);
    static void handleEmail(const QString& email, QWidget* parent);
    static void handlePhoneNumber(const QString& phone, QWidget* parent);
    static void handleWiFiConfig(const QString& config, QWidget* parent);
};
```

### 3. 设置系统扩展

#### 添加新的设置项
```cpp
class AppSettings {
public:
    // 现有设置
    static bool autoOpenUrls();
    static void setAutoOpenUrls(bool enable);
    
    // 新增设置项
    static QString defaultSaveLocation();
    static void setDefaultSaveLocation(const QString& path);
    
    static bool enableSoundNotification();
    static void setEnableSoundNotification(bool enable);
    
    static int recognitionTimeout();
    static void setRecognitionTimeout(int timeout);

private:
    static const QString SETTING_DEFAULT_SAVE_LOCATION;
    static const QString SETTING_SOUND_NOTIFICATION;
    static const QString SETTING_RECOGNITION_TIMEOUT;
};
```

## 🧪 测试指南

### 1. 单元测试框架

创建测试目录结构：
```
tests/
├── unit/
│   ├── test_qrcode_generator.cpp
│   ├── test_qrcode_recognizer.cpp
│   └── test_app_settings.cpp
├── integration/
│   ├── test_ui_integration.cpp
│   └── test_end_to_end.cpp
└── CMakeLists.txt
```

#### 示例单元测试
```cpp
// test_qrcode_generator.cpp
#include <QtTest/QtTest>
#include "core/QRCodeGenerator.h"

class TestQRCodeGenerator : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testBasicGeneration();
    void testUTF8Support();
    void testPDF417ChineseHandling();
    void testInvalidInput();
    void cleanupTestCase();

private:
    QRCodeGenerator* m_generator;
};

void TestQRCodeGenerator::initTestCase() {
    m_generator = new QRCodeGenerator(this);
}

void TestQRCodeGenerator::testBasicGeneration() {
    QString text = "Hello World";
    QPixmap result = m_generator->generateQRCode(text, BarcodeFormat::QRCode, 300);
    
    QVERIFY(!result.isNull());
    QCOMPARE(result.size(), QSize(300, 300));
}

void TestQRCodeGenerator::testUTF8Support() {
    QString text = "你好世界";
    QPixmap result = m_generator->generateQRCode(text, BarcodeFormat::QRCode, 300);
    
    QVERIFY(!result.isNull());
}

void TestQRCodeGenerator::testPDF417ChineseHandling() {
    QString text = "测试中文PDF417";
    
    // 应该不会崩溃
    QPixmap result = m_generator->generateQRCode(text, BarcodeFormat::PDF417, 300);
    QVERIFY(!result.isNull());
}

QTEST_MAIN(TestQRCodeGenerator)
#include "test_qrcode_generator.moc"
```

### 2. 集成测试

#### UI自动化测试
```cpp
// test_ui_integration.cpp
#include <QtTest/QtTest>
#include <QtWidgets>
#include "MainWindow.h"

class TestUIIntegration : public QObject {
    Q_OBJECT

private slots:
    void testGeneratorWorkflow();
    void testRecognizerWorkflow();
    void testSettingsDialog();

private:
    MainWindow* m_mainWindow;
};

void TestUIIntegration::testGeneratorWorkflow() {
    m_mainWindow = new MainWindow();
    m_mainWindow->show();
    
    // 切换到生成器标签页
    QTabWidget* tabWidget = m_mainWindow->findChild<QTabWidget*>();
    tabWidget->setCurrentIndex(0);
    
    // 输入文本
    QLineEdit* textInput = m_mainWindow->findChild<QLineEdit*>("textInput");
    QTest::keyClicks(textInput, "Test QR Code");
    
    // 点击生成按钮
    QPushButton* generateBtn = m_mainWindow->findChild<QPushButton*>("generateButton");
    QTest::mouseClick(generateBtn, Qt::LeftButton);
    
    // 验证结果
    QLabel* resultLabel = m_mainWindow->findChild<QLabel*>("resultDisplay");
    QVERIFY(!resultLabel->pixmap().isNull());
    
    delete m_mainWindow;
}
```

## 📊 性能优化指南

### 1. 生成性能优化

#### 异步生成
```cpp
class AsyncQRGenerator : public QObject {
    Q_OBJECT

public:
    void generateAsync(const QString& text, BarcodeFormat format, int size);

signals:
    void generationComplete(const QPixmap& result);
    void generationFailed(const QString& error);

private slots:
    void doGeneration();

private:
    QString m_text;
    BarcodeFormat m_format;
    int m_size;
};

// 使用QThread或QThreadPool
void AsyncQRGenerator::generateAsync(const QString& text, BarcodeFormat format, int size) {
    m_text = text;
    m_format = format;
    m_size = size;
    
    QTimer::singleShot(0, this, &AsyncQRGenerator::doGeneration);
}
```

#### 缓存机制
```cpp
class QRCodeCache {
public:
    static QPixmap getCachedQRCode(const QString& key);
    static void cacheQRCode(const QString& key, const QPixmap& pixmap);
    static void clearCache();

private:
    static QHash<QString, QPixmap> s_cache;
    static const int MAX_CACHE_SIZE = 100;
};

QString createCacheKey(const QString& text, BarcodeFormat format, int size) {
    return QString("%1_%2_%3").arg(text).arg(static_cast<int>(format)).arg(size);
}
```

### 2. 识别性能优化

#### 图像预处理
```cpp
QImage preprocessImage(const QImage& original) {
    QImage processed = original;
    
    // 缩放到合适尺寸（过大的图像会影响识别速度）
    if (processed.width() > 1920 || processed.height() > 1920) {
        processed = processed.scaled(1920, 1920, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    // 转换为灰度图（提高识别速度）
    if (processed.format() != QImage::Format_Grayscale8) {
        processed = processed.convertToFormat(QImage::Format_Grayscale8);
    }
    
    return processed;
}
```

#### 多线程识别
```cpp
class BatchRecognizer : public QObject {
    Q_OBJECT

public:
    void recognizeBatch(const QStringList& filePaths);

signals:
    void fileProcessed(const QString& filePath, const QRCodeResult& result);
    void batchComplete();

private:
    QThreadPool* m_threadPool;
};

class RecognitionTask : public QRunnable {
public:
    RecognitionTask(const QString& filePath, BatchRecognizer* parent);
    void run() override;

private:
    QString m_filePath;
    BatchRecognizer* m_parent;
};
```

## 🐛 调试指南

### 1. 调试宏和日志

#### 自定义调试宏
```cpp
// debug.h
#ifndef QR_DEBUG_H
#define QR_DEBUG_H

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(qrGenerator)
Q_DECLARE_LOGGING_CATEGORY(qrRecognizer)
Q_DECLARE_LOGGING_CATEGORY(qrUI)

#ifdef QT_DEBUG
#define QR_DEBUG(category, ...) qCDebug(category) << __VA_ARGS__
#define QR_WARNING(category, ...) qCWarning(category) << __VA_ARGS__
#define QR_CRITICAL(category, ...) qCCritical(category) << __VA_ARGS__
#else
#define QR_DEBUG(category, ...)
#define QR_WARNING(category, ...)
#define QR_CRITICAL(category, ...)
#endif

#endif // QR_DEBUG_H

// debug.cpp
#include "debug.h"

Q_LOGGING_CATEGORY(qrGenerator, "qr.generator")
Q_LOGGING_CATEGORY(qrRecognizer, "qr.recognizer")
Q_LOGGING_CATEGORY(qrUI, "qr.ui")
```

#### 使用示例
```cpp
#include "debug.h"

void QRCodeGenerator::generateQRCode(const QString& text) {
    QR_DEBUG(qrGenerator, "Generating QR code for text:" << text);
    
    try {
        // 生成逻辑
        QR_DEBUG(qrGenerator, "QR code generated successfully");
    } catch (const std::exception& e) {
        QR_CRITICAL(qrGenerator, "Failed to generate QR code:" << e.what());
    }
}
```

### 2. 内存泄漏检测

#### Valgrind (Linux)
```bash
valgrind --leak-check=full --show-leak-kinds=all ./QRcode_Generator_Recongniser
```

#### Application Verifier (Windows)
```cmd
# 启用Application Verifier
appverif.exe -enable Heaps -for QRcode_Generator_Recongniser.exe
```

### 3. 性能分析

#### QElapsedTimer使用
```cpp
#include <QElapsedTimer>

void measurePerformance() {
    QElapsedTimer timer;
    timer.start();
    
    // 执行要测量的代码
    generateQRCode(text);
    
    qint64 elapsed = timer.elapsed();
    QR_DEBUG(qrGenerator, "Generation took" << elapsed << "ms");
}
```

## 🚀 发布和部署

### 1. 版本管理

#### 版本号规范
使用语义化版本号：`MAJOR.MINOR.PATCH`
- MAJOR: 不兼容的API修改
- MINOR: 向后兼容的功能性新增
- PATCH: 向后兼容的问题修正

#### 版本信息管理
```cpp
// Version.h
#ifndef VERSION_H
#define VERSION_H

#define APP_VERSION_MAJOR 3
#define APP_VERSION_MINOR 0
#define APP_VERSION_PATCH 0
#define APP_VERSION_BUILD 1

#define APP_VERSION_STRING "3.0.0"
#define APP_BUILD_DATE __DATE__
#define APP_BUILD_TIME __TIME__

#endif // VERSION_H
```

### 2. 自动化构建

#### GitHub Actions配置
```yaml
# .github/workflows/build.yml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    strategy:
      matrix:
        os: [windows-latest, ubuntu-latest, macos-latest]
    
    runs-on: ${{ matrix.os }}
    
    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive
    
    - name: Install Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: '6.5.0'
    
    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Run Tests
      run: cd build && ctest --output-on-failure
    
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: QRCodeApp-${{ matrix.os }}
        path: build/QRcode_Generator_Recongniser*
```

### 3. 文档生成

#### Doxygen配置
```doxygen
# Doxyfile
PROJECT_NAME           = "QR Code Generator and Recognizer"
PROJECT_VERSION        = "3.0.0"
OUTPUT_DIRECTORY       = "docs/api"
INPUT                  = "src" "include"
RECURSIVE              = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
EXTRACT_ALL            = YES
```

---

这份开发者指南提供了参与项目开发所需的所有信息。如有疑问，请通过GitHub Issues联系项目维护者。
