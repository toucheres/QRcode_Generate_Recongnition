#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QCloseEvent>
#include <QImage>
#include <QPixmap>

// 前向声明
class GeneratorWidget;
class RecognizerWidget;
class CameraWidget;
class QRCodeGenerator;
class QRCodeRecognizer;

// 前向声明配置结构
namespace QRCodeConfig {
    struct GenerationConfig;
}

/**
 * @class MainWindow
 * @brief 重构后的主窗口类，负责整体界面布局和组件协调
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // 菜单操作
    void onNewProject();
    void onOpenFile();
    void onSaveProject();
    void onExit();
    void onAbout();
    void onSettings();
    
    // 生成相关
    void onGenerateQRCode();
    void onSaveQRCode(const QPixmap& pixmap);
    
    // 识别相关
    void onRecognizeQRCode(const QImage& image);
    void onRecognizeMultiFormat(const QImage& image);
    
    // 摄像头相关
    void onQRCodeDetected(const QString& text);
    void onCameraError(const QString& error);
    
    // 主题相关
    void onThemeChanged(bool isDark);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void updateWindowTitle(const QString& subtitle = QString());
    void applyThemeStyles();
    
    // 文件操作
    bool saveQRCodeToFile(const QPixmap& pixmap);
    void loadSettings();
    void saveSettings();

private:
    // UI组件
    QTabWidget* m_tabWidget;
    
    // 功能组件
    GeneratorWidget* m_generatorWidget;
    RecognizerWidget* m_recognizerWidget;
    CameraWidget* m_cameraWidget;
    
    // 核心组件
    QRCodeGenerator* m_generator;
    QRCodeRecognizer* m_recognizer;
    
    // 菜单和状态栏
    QMenuBar* m_menuBar;
    QStatusBar* m_statusBar;
    
    // 状态
    QString m_currentProjectFile;
    bool m_hasUnsavedChanges;
};
