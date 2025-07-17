#include "gui/RecognizerWidget.h"
#include "utils/AppUtils.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QTextStream>
#include <QUrl>

RecognizerWidget::RecognizerWidget(QWidget* parent)
    : BaseWidget(parent), m_recognizer(new QRCodeRecognizer(this)), m_requestCounter(0),
      m_networkManager(new QNetworkAccessManager(this)), m_currentReply(nullptr),
      m_urlValidationTimer(new QTimer(this))
{
    setupUI();
    setAcceptDrops(true);

    // 连接识别器信号
    connect(m_recognizer, &QRCodeRecognizer::recognitionCompleted, this,
            &RecognizerWidget::onRecognitionCompleted);
    connect(m_recognizer, &QRCodeRecognizer::recognitionFailed, this,
            &RecognizerWidget::onRecognitionFailed);

    // 配置URL验证定时器
    m_urlValidationTimer->setSingleShot(true);
    m_urlValidationTimer->setInterval(500); // 500ms延迟验证
    connect(m_urlValidationTimer, &QTimer::timeout, this, &RecognizerWidget::onUrlInputChanged);
}

void RecognizerWidget::setConfig(const QRCodeRecognizer::RecognitionConfig& config)
{
    m_tryHarderCheckBox->setChecked(config.tryHarder);
    m_tryRotateCheckBox->setChecked(config.tryRotate);
    m_fastModeCheckBox->setChecked(config.fastMode);
    m_maxSymbolsSpinBox->setValue(config.maxSymbols);

    m_recognizer->setConfig(config);
}

QRCodeRecognizer::RecognitionConfig RecognizerWidget::getConfig() const
{
    QRCodeRecognizer::RecognitionConfig config;
    config.tryHarder = m_tryHarderCheckBox->isChecked();
    config.tryRotate = m_tryRotateCheckBox->isChecked();
    config.fastMode = m_fastModeCheckBox->isChecked();
    config.maxSymbols = m_maxSymbolsSpinBox->value();
    return config;
}

void RecognizerWidget::showRecognitionResult(const QRCodeRecognizer::RecognitionResult& result)
{
    updateResultDisplay(result);
    
    // 在原图像上绘制识别轮廓
    if (result.isValid && !m_currentPixmap.isNull()) {
        QPixmap contourImage = m_recognizer->drawContour(m_currentPixmap, result);
        
        // 更新显示的图像
        QPixmap displayPixmap = contourImage.scaled(m_imageLabel->size(), 
                                                   Qt::KeepAspectRatio, 
                                                   Qt::SmoothTransformation);
        m_imageLabel->setPixmap(displayPixmap);
        
        // 保存带轮廓的图像，用于后续操作
        m_currentPixmapWithContour = contourImage;
    }
    
    m_statusLabel->setText("识别完成");
    m_progressBar->setVisible(false);
}

void RecognizerWidget::showError(const QString& error)
{
    m_statusLabel->setText(QString("错误: %1").arg(error));
    m_progressBar->setVisible(false);

    // 在结果区域显示错误
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_resultsTextEdit->append(
        QString("[%1] <span style='color: red;'>错误: %2</span>").arg(timestamp).arg(error));
}

void RecognizerWidget::showMultiFormatResults(
    const QList<QRCodeRecognizer::RecognitionResult>& results)
{
    if (results.isEmpty())
    {
        showError("未识别到任何条码格式");
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_resultsTextEdit->append(QString("[%1] <span style='color: green;'>识别到 %2 种格式：</span>")
                                  .arg(timestamp)
                                  .arg(results.size()));

    // 按流行度显示所有识别结果
    for (int i = 0; i < results.size(); ++i)
    {
        const auto& result = results[i];
        QString rank =
            i == 0 ? "🥇" : (i == 1 ? "🥈" : (i == 2 ? "🥉" : QString("第%1位").arg(i + 1)));

        m_resultsTextEdit->append(
            QString("  %1 <b>%2</b>: %3").arg(rank).arg(result.format).arg(result.text));
    }

    // 更新单个结果显示（显示最流行的）
    updateResultDisplay(results.first());
    
    // 在原图像上绘制识别轮廓（显示最流行的结果）
    if (!results.isEmpty() && results.first().isValid && !m_currentPixmap.isNull()) {
        QPixmap contourImage = m_recognizer->drawContour(m_currentPixmap, results.first());
        
        // 更新显示的图像
        QPixmap displayPixmap = contourImage.scaled(m_imageLabel->size(), 
                                                   Qt::KeepAspectRatio, 
                                                   Qt::SmoothTransformation);
        m_imageLabel->setPixmap(displayPixmap);
        
        // 保存带轮廓的图像
        m_currentPixmapWithContour = contourImage;
    }
    
    m_statusLabel->setText(QString("识别完成 - 找到%1种格式").arg(results.size()));
    m_progressBar->setVisible(false);
}

void RecognizerWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        bool hasImageFile = false;
        for (const QUrl& url : event->mimeData()->urls())
        {
            QString filePath = url.toLocalFile();
            if (AppUtils::isImageFile(filePath))
            {
                hasImageFile = true;
                break;
            }
        }
        if (hasImageFile)
        {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void RecognizerWidget::dropEvent(QDropEvent* event)
{
    QStringList imagePaths;
    for (const QUrl& url : event->mimeData()->urls())
    {
        QString filePath = url.toLocalFile();
        if (AppUtils::isImageFile(filePath))
        {
            imagePaths.append(filePath);
        }
    }

    if (!imagePaths.isEmpty())
    {
        if (imagePaths.size() == 1)
        {
            loadImageFromFile(imagePaths.first());
        }
        else
        {
            processImageList(imagePaths);
        }
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void RecognizerWidget::onSelectImageClicked()
{
    QStringList fileNames =
        QFileDialog::getOpenFileNames(this, "选择图片文件", "", AppUtils::getImageFileFilter());

    if (!fileNames.isEmpty())
    {
        if (fileNames.size() == 1)
        {
            loadImageFromFile(fileNames.first());
        }
        else
        {
            processImageList(fileNames);
        }
    }
}

void RecognizerWidget::onRecognizeClicked()
{
    if (m_currentImage.isNull())
    {
        QMessageBox::warning(this, "警告", "请先选择图片！");
        return;
    }

    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定进度
    m_statusLabel->setText("正在识别...");

    // 异步识别
    QRCodeRecognizer::RecognitionConfig config = getConfig();
    m_recognizer->recognizeAsync(m_currentImage, ++m_requestCounter, config);
}

void RecognizerWidget::onMultiFormatClicked()
{
    if (m_currentImage.isNull())
    {
        QMessageBox::warning(this, "警告", "请先选择图片！");
        return;
    }

    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定进度
    m_statusLabel->setText("正在识别多种格式...");

    // 发出多格式识别信号
    emit multiFormatRecognizeRequested(m_currentImage);
}

void RecognizerWidget::onClearResultsClicked()
{
    m_resultsTextEdit->clear();
    m_results.clear();
    m_statusLabel->setText("结果已清空");
}

void RecognizerWidget::onSaveResultsClicked()
{
    if (m_results.isEmpty())
    {
        QMessageBox::warning(this, "警告", "没有识别结果可保存！");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "保存识别结果",
        QString("recognition_results_%1.txt")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)");

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);

            out << "二维码识别结果报告\n";
            out << "生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                << "\n";
            out << "识别数量: " << m_results.size() << "\n\n";

            for (int i = 0; i < m_results.size(); ++i)
            {
                const auto& result = m_results[i];
                out << "=== 结果 " << (i + 1) << " ===\n";
                out << "内容: " << result.text << "\n";
                out << "格式: " << result.format << "\n";
                out << "置信度: " << QString::number(result.confidence, 'f', 2) << "\n\n";
            }

            m_statusLabel->setText("结果已保存到: " + fileName);
        }
        else
        {
            QMessageBox::critical(this, "错误", "无法保存文件！");
        }
    }
}

void RecognizerWidget::onConfigChanged()
{
    QRCodeRecognizer::RecognitionConfig config = getConfig();
    emit configChanged(config);
}

void RecognizerWidget::onRecognitionCompleted(const QRCodeRecognizer::RecognitionResult& result,
                                              int requestId)
{
    if (requestId == m_requestCounter)
    {
        showRecognitionResult(result);
    }
}

void RecognizerWidget::onRecognitionFailed(const QString& error, int requestId)
{
    if (requestId == m_requestCounter)
    {
        showError(error);
    }
}

void RecognizerWidget::setupUI()
{
    setWindowTitle("二维码识别器");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);

    // 左侧：图片区域
    QWidget* imageWidget = new QWidget();
    imageWidget->setMinimumWidth(400);
    QVBoxLayout* imageLayout = createVBoxLayout(imageWidget);

    imageLayout->addWidget(createLabel("图片预览:"));

    m_imageScrollArea = new QScrollArea();
    m_imageLabel = new QLabel();
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setText("拖放图片到此处或点击按钮选择图片");
    m_imageLabel->setMinimumSize(300, 300);
    m_imageLabel->setStyleSheet("QLabel {"
                                "    border: 2px dashed #aaaaaa;"
                                "    border-radius: 8px;"
                                "    background-color: #f5f5f5;"
                                "    color: #666666;"
                                "    font-size: 14px;"
                                "}");
    m_imageScrollArea->setWidget(m_imageLabel);
    m_imageScrollArea->setWidgetResizable(true);
    imageLayout->addWidget(m_imageScrollArea);

    // 按钮区域
    QHBoxLayout* buttonLayout = createHBoxLayout(nullptr, 0);
    m_selectImageButton = createButton("选择图片");
    m_recognizeButton = createButton("QR识别");
    m_multiFormatButton = createButton("多格式识别");
    m_recognizeButton->setEnabled(false);
    m_multiFormatButton->setEnabled(false);

    // 设置按钮样式和提示
    m_recognizeButton->setToolTip("识别二维码格式");
    m_multiFormatButton->setToolTip("识别所有支持的条码格式，按流行度排序显示");
    m_multiFormatButton->setStyleSheet("QPushButton {"
                                       "background-color: #4CAF50;"
                                       "color: white;"
                                       "font-weight: bold;"
                                       "}");

    buttonLayout->addWidget(m_selectImageButton);
    buttonLayout->addWidget(m_recognizeButton);
    buttonLayout->addWidget(m_multiFormatButton);
    buttonLayout->addStretch();
    imageLayout->addLayout(buttonLayout);

    // 网络图片输入区域
    QGroupBox* urlGroup = createGroupBox("网络图片");
    QVBoxLayout* urlLayout = createVBoxLayout(urlGroup, 10, 8);

    urlLayout->addWidget(createLabel("图片链接:"));
    m_urlLineEdit = new QLineEdit();
    m_urlLineEdit->setPlaceholderText("请输入图片URL，如：https://example.com/qr.png");
    m_urlLineEdit->setToolTip("支持 HTTP/HTTPS 图片链接\n"
                              "支持格式：JPG、PNG、BMP、GIF 等");
    urlLayout->addWidget(m_urlLineEdit);

    QHBoxLayout* urlButtonLayout = createHBoxLayout(nullptr, 0);
    m_loadFromUrlButton = createButton("加载并识别");
    m_loadFromUrlButton->setEnabled(false);
    m_loadFromUrlButton->setStyleSheet("QPushButton {"
                                       "background-color: #2196F3;"
                                       "color: white;"
                                       "font-weight: bold;"
                                       "}");

    m_urlStatusLabel = new QLabel();
    m_urlStatusLabel->setStyleSheet("QLabel { color: #666; font-size: 12px; }");

    urlButtonLayout->addWidget(m_loadFromUrlButton);
    urlButtonLayout->addStretch();
    urlLayout->addLayout(urlButtonLayout);
    urlLayout->addWidget(m_urlStatusLabel);

    imageLayout->addWidget(urlGroup);

    // 右侧：配置和结果
    QWidget* rightWidget = new QWidget();
    rightWidget->setMaximumWidth(350);
    QVBoxLayout* rightLayout = createVBoxLayout(rightWidget);

    // 配置区域
    m_configGroup = createGroupBox("识别配置");
    QVBoxLayout* configLayout = createVBoxLayout(m_configGroup, 10, 8);

    m_tryHarderCheckBox = new QCheckBox("严格模式 (更准确但更慢)");
    m_tryRotateCheckBox = new QCheckBox("尝试旋转识别");
    m_tryRotateCheckBox->setChecked(true);
    m_fastModeCheckBox = new QCheckBox("快速模式 (更快但精度较低)");

    configLayout->addWidget(m_tryHarderCheckBox);
    configLayout->addWidget(m_tryRotateCheckBox);
    configLayout->addWidget(m_fastModeCheckBox);

    QHBoxLayout* maxSymbolsLayout = createHBoxLayout(nullptr, 0);
    maxSymbolsLayout->addWidget(createLabel("最大识别数量:"));
    m_maxSymbolsSpinBox = new QSpinBox();
    m_maxSymbolsSpinBox->setRange(1, 10);
    m_maxSymbolsSpinBox->setValue(1);
    maxSymbolsLayout->addWidget(m_maxSymbolsSpinBox);
    maxSymbolsLayout->addStretch();
    configLayout->addLayout(maxSymbolsLayout);

    rightLayout->addWidget(m_configGroup);

    // 结果区域
    m_resultsGroup = createGroupBox("识别结果");
    QVBoxLayout* resultsLayout = createVBoxLayout(m_resultsGroup, 10, 8);

    m_resultsTextEdit = new QTextEdit();
    m_resultsTextEdit->setMaximumHeight(200);
    m_resultsTextEdit->setReadOnly(true);
    m_resultsTextEdit->setPlaceholderText("识别结果将显示在这里...");
    resultsLayout->addWidget(m_resultsTextEdit);

    QHBoxLayout* resultButtonLayout = createHBoxLayout(nullptr, 0);
    m_clearResultsButton = createButton("清空结果");
    m_saveResultsButton = createButton("保存结果");
    resultButtonLayout->addWidget(m_clearResultsButton);
    resultButtonLayout->addWidget(m_saveResultsButton);
    resultsLayout->addLayout(resultButtonLayout);

    rightLayout->addWidget(m_resultsGroup);

    // 状态栏
    m_statusLabel = createLabel("准备就绪");
    m_statusLabel->setStyleSheet("color: #666666; font-size: 8pt;");
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    rightLayout->addWidget(m_statusLabel);
    rightLayout->addWidget(m_progressBar);

    rightLayout->addStretch();

    // 添加到主布局
    mainLayout->addWidget(imageWidget);
    mainLayout->addWidget(rightWidget);

    // 连接信号
    connect(m_selectImageButton, &QPushButton::clicked, this,
            &RecognizerWidget::onSelectImageClicked);
    connect(m_recognizeButton, &QPushButton::clicked, this, &RecognizerWidget::onRecognizeClicked);
    connect(m_multiFormatButton, &QPushButton::clicked, this,
            &RecognizerWidget::onMultiFormatClicked);
    connect(m_clearResultsButton, &QPushButton::clicked, this,
            &RecognizerWidget::onClearResultsClicked);
    connect(m_saveResultsButton, &QPushButton::clicked, this,
            &RecognizerWidget::onSaveResultsClicked);

    // 网络相关连接
    connect(m_loadFromUrlButton, &QPushButton::clicked, this,
            &RecognizerWidget::onLoadFromUrlClicked);
    connect(m_urlLineEdit, &QLineEdit::textChanged,
            [this]()
            {
                m_urlValidationTimer->start(); // 重启定时器
            });

    connect(m_tryHarderCheckBox, &QCheckBox::toggled, this, &RecognizerWidget::onConfigChanged);
    connect(m_tryRotateCheckBox, &QCheckBox::toggled, this, &RecognizerWidget::onConfigChanged);
    connect(m_fastModeCheckBox, &QCheckBox::toggled, this, &RecognizerWidget::onConfigChanged);
    connect(m_maxSymbolsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &RecognizerWidget::onConfigChanged);
}

void RecognizerWidget::updateImagePreview(const QImage& image)
{
    if (image.isNull())
    {
        return;
    }

    m_currentImage = image;
    m_currentPixmap = QPixmap::fromImage(image);

    // 缩放显示
    QPixmap displayPixmap =
        m_currentPixmap.scaled(m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageLabel->setPixmap(displayPixmap);
    m_recognizeButton->setEnabled(true);
    m_multiFormatButton->setEnabled(true);

    m_statusLabel->setText(QString("图片已加载 (%1x%2)").arg(image.width()).arg(image.height()));
}

void RecognizerWidget::updateResultDisplay(const QRCodeRecognizer::RecognitionResult& result)
{
    if (!result.isValid)
    {
        return;
    }

    m_results.append(result);

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString resultHtml = QString("<div style='margin: 5px 0; padding: 8px; background-color: "
                                 "#f0f0f0; border-radius: 4px;'>"
                                 "<b>[%1]</b> 识别成功<br/>"
                                 "<b>内容:</b> %2<br/>"
                                 "<b>格式:</b> %3<br/>"
                                 "<b>置信度:</b> %4"
                                 "</div>")
                             .arg(timestamp)
                             .arg(result.text.toHtmlEscaped())
                             .arg(result.format)
                             .arg(QString::number(result.confidence, 'f', 2));

    m_resultsTextEdit->append(resultHtml);

    // 自动复制到剪贴板（如果只有一个结果）
    if (m_results.size() == 1)
    {
        QApplication::clipboard()->setText(result.text);
        m_statusLabel->setText("识别完成，内容已复制到剪贴板");
    }
}

void RecognizerWidget::loadImageFromFile(const QString& filePath)
{
    QImage image(filePath);
    if (!image.isNull())
    {
        updateImagePreview(image);
    }
    else
    {
        QMessageBox::warning(this, "错误", "无法加载图片: " + filePath);
    }
}

void RecognizerWidget::processImageList(const QStringList& filePaths)
{
    if (filePaths.isEmpty())
    {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "批量识别", QString("选择了 %1 个图片文件，是否要批量识别？").arg(filePaths.size()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        m_progressBar->setVisible(true);
        m_progressBar->setRange(0, filePaths.size());

        QRCodeRecognizer::RecognitionConfig config = getConfig();

        for (int i = 0; i < filePaths.size(); ++i)
        {
            QImage image(filePaths[i]);
            if (!image.isNull())
            {
                auto result = m_recognizer->recognizeSync(image, config);
                if (result.isValid)
                {
                    updateResultDisplay(result);
                }
                else
                {
                    QString errorMsg = QString("文件 %1: %2")
                                           .arg(QFileInfo(filePaths[i]).fileName())
                                           .arg(m_recognizer->getLastError());
                    showError(errorMsg);
                }
            }
            m_progressBar->setValue(i + 1);
            QApplication::processEvents();
        }

        m_progressBar->setVisible(false);
        m_statusLabel->setText(QString("批量识别完成，共处理 %1 个文件").arg(filePaths.size()));
    }
    else
    {
        // 只加载第一个图片
        loadImageFromFile(filePaths.first());
    }
}

void RecognizerWidget::applyDefaultStyles()
{
    BaseWidget::applyDefaultStyles();

    // 应用特定样式
    setStyleSheet(styleSheet() + "QScrollArea { border: 1px solid #cccccc; border-radius: 4px; }"
                                 "QTextEdit { border: 1px solid #cccccc; border-radius: 4px; }");
}

void RecognizerWidget::onLoadFromUrlClicked()
{
    QString url = m_urlLineEdit->text().trimmed();
    if (url.isEmpty())
    {
        showError("请输入图片URL");
        return;
    }

    // 取消当前下载
    if (m_currentReply)
    {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    // 开始下载
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "QRCode Recognition Tool 1.0");
    request.setRawHeader("Accept", "image/*");

    m_currentReply = m_networkManager->get(request);

    // 连接信号
    connect(m_currentReply, &QNetworkReply::finished, this,
            &RecognizerWidget::onNetworkImageDownloaded);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this,
            &RecognizerWidget::onDownloadProgress);
    connect(m_currentReply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred), this,
            &RecognizerWidget::onNetworkError);

    // 更新UI状态
    m_loadFromUrlButton->setEnabled(false);
    m_urlStatusLabel->setText("正在下载图片...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_statusLabel->setText("正在从网络加载图片...");
}

void RecognizerWidget::onUrlInputChanged()
{
    QString url = m_urlLineEdit->text().trimmed();

    if (url.isEmpty())
    {
        m_loadFromUrlButton->setEnabled(false);
        m_urlStatusLabel->setText("");
        return;
    }

    // 验证URL格式
    QUrl qurl(url);
    if (!qurl.isValid() || qurl.scheme().isEmpty())
    {
        m_loadFromUrlButton->setEnabled(false);
        m_urlStatusLabel->setText("❌ 无效的URL格式");
        m_urlStatusLabel->setStyleSheet("QLabel { color: #f44336; font-size: 12px; }");
        return;
    }

    // 检查是否为HTTP/HTTPS
    if (qurl.scheme() != "http" && qurl.scheme() != "https")
    {
        m_loadFromUrlButton->setEnabled(false);
        m_urlStatusLabel->setText("❌ 仅支持HTTP/HTTPS协议");
        m_urlStatusLabel->setStyleSheet("QLabel { color: #f44336; font-size: 12px; }");
        return;
    }

    // 检查文件扩展名（可选）
    QString path = qurl.path().toLower();
    QStringList imageExtensions = {".jpg", ".jpeg", ".png", ".bmp", ".gif", ".webp"};
    bool hasImageExt = false;
    for (const QString& ext : imageExtensions)
    {
        if (path.endsWith(ext))
        {
            hasImageExt = true;
            break;
        }
    }

    m_loadFromUrlButton->setEnabled(true);
    if (hasImageExt)
    {
        m_urlStatusLabel->setText("✅ URL格式正确");
        m_urlStatusLabel->setStyleSheet("QLabel { color: #4CAF50; font-size: 12px; }");
    }
    else
    {
        m_urlStatusLabel->setText("⚠️ URL格式正确，但可能不是图片");
        m_urlStatusLabel->setStyleSheet("QLabel { color: #FF9800; font-size: 12px; }");
    }
}

void RecognizerWidget::onNetworkImageDownloaded()
{
    if (!m_currentReply)
        return;

    // 恢复UI状态
    m_loadFromUrlButton->setEnabled(true);
    m_progressBar->setVisible(false);

    if (m_currentReply->error() != QNetworkReply::NoError)
    {
        m_urlStatusLabel->setText("❌ 下载失败");
        m_urlStatusLabel->setStyleSheet("QLabel { color: #f44336; font-size: 12px; }");
        showError(QString("网络错误: %1").arg(m_currentReply->errorString()));
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    // 读取图片数据
    QByteArray imageData = m_currentReply->readAll();
    QImage image;

    if (!image.loadFromData(imageData))
    {
        m_urlStatusLabel->setText("❌ 不是有效的图片");
        m_urlStatusLabel->setStyleSheet("QLabel { color: #f44336; font-size: 12px; }");
        showError("下载的文件不是有效的图片格式");
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        return;
    }

    // 成功加载图片
    m_urlStatusLabel->setText("✅ 图片加载成功");
    m_urlStatusLabel->setStyleSheet("QLabel { color: #4CAF50; font-size: 12px; }");

    // 更新图片预览
    m_currentImage = image;
    updateImagePreview(image);

    // 启用识别按钮
    m_recognizeButton->setEnabled(true);
    m_multiFormatButton->setEnabled(true);

    // 自动开始多格式识别
    m_statusLabel->setText("图片加载成功，开始识别...");

    // 使用多格式识别
    QRCodeRecognizer::RecognitionConfig config = getConfig();
    config.maxSymbols = 10; // 网络图片可能包含多个条码

    QList<QRCodeRecognizer::RecognitionResult> results =
        m_recognizer->recognizeMultiFormat(m_currentImage, config);

    if (!results.isEmpty())
    {
        showMultiFormatResults(results);
    }
    else
    {
        showError("未在图片中识别到任何条码");
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void RecognizerWidget::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_currentReply)
        return;

    m_loadFromUrlButton->setEnabled(true);
    m_progressBar->setVisible(false);
    m_urlStatusLabel->setText("❌ 网络错误");
    m_urlStatusLabel->setStyleSheet("QLabel { color: #f44336; font-size: 12px; }");

    QString errorMessage;
    switch (error)
    {
    case QNetworkReply::ConnectionRefusedError:
        errorMessage = "连接被拒绝";
        break;
    case QNetworkReply::RemoteHostClosedError:
        errorMessage = "远程主机关闭连接";
        break;
    case QNetworkReply::HostNotFoundError:
        errorMessage = "找不到主机";
        break;
    case QNetworkReply::TimeoutError:
        errorMessage = "连接超时";
        break;
    case QNetworkReply::SslHandshakeFailedError:
        errorMessage = "SSL握手失败";
        break;
    case QNetworkReply::ContentNotFoundError:
        errorMessage = "内容未找到(404)";
        break;
    default:
        errorMessage = QString("网络错误(%1)").arg(static_cast<int>(error));
        break;
    }

    showError(errorMessage);
}

void RecognizerWidget::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0)
    {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        m_progressBar->setValue(progress);
        m_urlStatusLabel->setText(QString("下载中... %1%").arg(progress));
    }
    else
    {
        m_urlStatusLabel->setText(QString("已下载 %1 字节").arg(bytesReceived));
    }
}
