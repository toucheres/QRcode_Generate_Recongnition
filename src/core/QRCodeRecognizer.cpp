#include "core/QRCodeRecognizer.h"
#include <QDebug>
#include <QPainter>
#include <QDateTime>
#include <QApplication>

// ZXing includes
#include "ImageView.h"
#include "BarcodeFormat.h"

QRCodeRecognizer::QRCodeRecognizer(QObject* parent)
    : QObject(parent)
    , m_processTimer(new QTimer(this))
{
    m_processTimer->setSingleShot(false);
    m_processTimer->setInterval(PROCESSING_INTERVAL);
    connect(m_processTimer, &QTimer::timeout, this, &QRCodeRecognizer::processRecognitionQueue);
}

QRCodeRecognizer::~QRCodeRecognizer()
{
    m_processTimer->stop();
}

QRCodeRecognizer::RecognitionResult QRCodeRecognizer::recognizeSync(const QImage& image, const RecognitionConfig& config)
{
    if (image.isNull()) {
        m_lastError = "图像无效";
        return RecognitionResult();
    }

    return performRecognition(image, config);
}

void QRCodeRecognizer::recognizeAsync(const QImage& image, int requestId, const RecognitionConfig& config)
{
    if (image.isNull()) {
        emit recognitionFailed("图像无效", requestId);
        return;
    }

    QMutexLocker locker(&m_queueMutex);
    
    // 限制队列大小
    if (m_requestQueue.size() >= MAX_QUEUE_SIZE) {
        m_requestQueue.dequeue(); // 移除最老的请求
    }
    
    AsyncRequest request;
    request.image = image;
    request.requestId = requestId;
    request.config = config;
    request.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    m_requestQueue.enqueue(request);
    
    // 启动处理定时器
    if (!m_processTimer->isActive()) {
        m_processTimer->start();
    }
}

QRCodeRecognizer::RecognitionResult QRCodeRecognizer::recognizeFromPixmap(const QPixmap& pixmap, const RecognitionConfig& config)
{
    if (pixmap.isNull()) {
        m_lastError = "QPixmap无效";
        return RecognitionResult();
    }

    return recognizeSync(pixmap.toImage(), config);
}

QPixmap QRCodeRecognizer::drawContour(const QPixmap& originalImage, const RecognitionResult& result)
{
    if (originalImage.isNull() || !result.isValid) {
        return originalImage;
    }

    QPixmap resultImage = originalImage;
    QPainter painter(&resultImage);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 设置绘制样式
    QPen pen(Qt::red, 3);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    // 绘制二维码轮廓
    const auto& pos = result.position;
    QPolygon polygon;
    polygon << QPoint(pos.topLeft().x, pos.topLeft().y)
            << QPoint(pos.topRight().x, pos.topRight().y)
            << QPoint(pos.bottomRight().x, pos.bottomRight().y)
            << QPoint(pos.bottomLeft().x, pos.bottomLeft().y);
    
    painter.drawPolygon(polygon);

    // 绘制角点
    painter.setBrush(Qt::red);
    int pointSize = 8;
    for (const auto& point : {pos.topLeft(), pos.topRight(), pos.bottomRight(), pos.bottomLeft()}) {
        painter.drawEllipse(QPoint(point.x, point.y), pointSize/2, pointSize/2);
    }

    // 绘制识别信息
    if (!result.text.isEmpty()) {
        painter.setPen(Qt::white);
        painter.setBrush(QColor(0, 0, 0, 180));
        
        QFontMetrics fm(painter.font());
        QString displayText = result.text.length() > 50 ? 
                             result.text.left(47) + "..." : result.text;
        QRect textRect = fm.boundingRect(displayText);
        textRect.adjust(-5, -2, 5, 2);
        textRect.moveTopLeft(QPoint(pos.topLeft().x, pos.topLeft().y - textRect.height() - 5));
        
        painter.drawRect(textRect);
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, displayText);
    }

    return resultImage;
}

QString QRCodeRecognizer::getLastError() const
{
    return m_lastError;
}

void QRCodeRecognizer::setConfig(const RecognitionConfig& config)
{
    m_defaultConfig = config;
}

void QRCodeRecognizer::processRecognitionQueue()
{
    QMutexLocker locker(&m_queueMutex);
    
    if (m_requestQueue.isEmpty() || m_processing) {
        if (m_requestQueue.isEmpty()) {
            m_processTimer->stop();
        }
        return;
    }
    
    AsyncRequest request = m_requestQueue.dequeue();
    locker.unlock();
    
    m_processing = true;
    
    // 在主线程中执行识别（如果需要可以移到工作线程）
    QApplication::processEvents(); // 保持界面响应
    
    RecognitionResult result = performRecognition(request.image, request.config);
    
    m_processing = false;
    
    if (result.isValid) {
        emit recognitionCompleted(result, request.requestId);
    } else {
        emit recognitionFailed(m_lastError, request.requestId);
    }
}

QRCodeRecognizer::RecognitionResult QRCodeRecognizer::performRecognition(const QImage& image, const RecognitionConfig& config)
{
    // 使用多格式识别，然后返回第一个（最流行的）结果
    QList<RecognitionResult> results = recognizeMultiFormat(image, config);
    
    if (results.isEmpty()) {
        return RecognitionResult();
    }
    
    return results.first();
}

ZXing::ReaderOptions QRCodeRecognizer::convertConfig(const RecognitionConfig& config)
{
    ZXing::ReaderOptions options;
    
    // 设置支持所有主要条码格式
    options.setFormats(ZXing::BarcodeFormat::QRCode | 
                      ZXing::BarcodeFormat::MicroQRCode |
                      ZXing::BarcodeFormat::DataMatrix |
                      ZXing::BarcodeFormat::PDF417 |
                      ZXing::BarcodeFormat::Code128 |
                      ZXing::BarcodeFormat::Code39 |
                      ZXing::BarcodeFormat::Code93 |
                      ZXing::BarcodeFormat::Codabar |
                      ZXing::BarcodeFormat::EAN8 |
                      ZXing::BarcodeFormat::EAN13 |
                      ZXing::BarcodeFormat::UPCA |
                      ZXing::BarcodeFormat::UPCE |
                      ZXing::BarcodeFormat::ITF |
                      ZXing::BarcodeFormat::Aztec |
                      ZXing::BarcodeFormat::MaxiCode);
    
    // 设置其他选项
    options.setTryHarder(config.tryHarder);
    options.setTryRotate(config.tryRotate);
    options.setMaxNumberOfSymbols(config.maxSymbols);
    
    // 根据快速模式调整其他参数
    if (config.fastMode) {
        options.setTryHarder(false);
        options.setTryRotate(false);
    }
    
    return options;
}

QImage QRCodeRecognizer::preprocessImage(const QImage& image)
{
    QImage processed = image;
    
    // 转换为合适的格式
    if (processed.format() != QImage::Format_RGBX8888 && 
        processed.format() != QImage::Format_RGB32) {
        processed = processed.convertToFormat(QImage::Format_RGBX8888);
    }
    
    // 如果图像太大，进行缩放以提高处理速度
    const int maxDimension = 1920;
    if (processed.width() > maxDimension || processed.height() > maxDimension) {
        processed = processed.scaled(maxDimension, maxDimension, 
                                   Qt::KeepAspectRatio, 
                                   Qt::SmoothTransformation);
    }
    
    return processed;
}

QList<QRCodeRecognizer::RecognitionResult> QRCodeRecognizer::recognizeMultiFormat(const QImage& image, const RecognitionConfig& config)
{
    QList<RecognitionResult> results;
    
    if (image.isNull()) {
        m_lastError = "图像无效";
        return results;
    }

    try {
        // 预处理图像
        QImage processedImage = preprocessImage(image);
        
        // 创建ZXing的ImageView
        ZXing::ImageView imageView(processedImage.bits(), 
                                  processedImage.width(), 
                                  processedImage.height(),
                                  ZXing::ImageFormat::RGBA);

        // 设置识别选项
        ZXing::ReaderOptions options = convertConfig(config);
        
        // 执行识别
        auto zxingResults = ZXing::ReadBarcodes(imageView, options);
        
        if (zxingResults.empty()) {
            m_lastError = "未找到任何条码";
            return results;
        }

        // 转换结果并按流行度排序
        for (const auto& barcode : zxingResults) {
            if (barcode.isValid()) {
                RecognitionResult result;
                result.text = QString::fromUtf8(barcode.text().c_str());
                result.isValid = true;
                result.position = barcode.position();
                result.format = QString::fromStdString(ToString(barcode.format()));
                result.confidence = 1.0; // ZXing不直接提供置信度
                
                results.append(result);
            }
        }
        
        // 按流行度排序
        std::sort(results.begin(), results.end(), [this](const RecognitionResult& a, const RecognitionResult& b) {
            // 首先按流行度得分排序，然后按置信度
            int scoreA = getFormatPopularityScore(ZXing::BarcodeFormatFromString(a.format.toStdString()));
            int scoreB = getFormatPopularityScore(ZXing::BarcodeFormatFromString(b.format.toStdString()));
            
            if (scoreA != scoreB) {
                return scoreA > scoreB; // 得分高的排在前面
            }
            return a.confidence > b.confidence; // 置信度高的排在前面
        });
        
        m_lastError.clear();
        return results;
    }
    catch (const std::exception& e) {
        m_lastError = QString("识别异常: %1").arg(e.what());
        qDebug() << m_lastError;
        return results;
    }
}

QStringList QRCodeRecognizer::getSupportedFormats() const
{
    return {
        "QR Code - 二维码 (最流行)",
        "EAN-13 - 商品条码 (广泛使用)",
        "EAN-8 - 商品条码 (小包装)",
        "UPC-A - 美国商品码",
        "UPC-E - 美国商品码 (紧凑版)",
        "Code 128 - 一维条码 (物流)",
        "Code 39 - 三九码 (工业)",
        "Data Matrix - 数据矩阵 (高密度)",
        "PDF417 - 大容量二维码",
        "Aztec - 阿兹特克码 (交通)",
        "ITF - 交错25码 (数字)",
        "Code 93 - 九三码",
        "Codabar - 库德巴码 (医疗)",
        "Micro QR Code - 微型二维码",
        "MaxiCode - 最大码 (邮政)"
    };
}

QList<ZXing::BarcodeFormat> QRCodeRecognizer::getFormatsOrderedByPopularity() const
{
    return {
        ZXing::BarcodeFormat::QRCode,        // 100分 - 最流行
        ZXing::BarcodeFormat::EAN13,         // 90分 - 商品条码标准
        ZXing::BarcodeFormat::EAN8,          // 85分 - 小包装商品
        ZXing::BarcodeFormat::UPCA,          // 80分 - 北美标准
        ZXing::BarcodeFormat::UPCE,          // 75分 - 紧凑版
        ZXing::BarcodeFormat::Code128,       // 70分 - 物流常用
        ZXing::BarcodeFormat::Code39,        // 65分 - 工业应用
        ZXing::BarcodeFormat::DataMatrix,    // 60分 - 高密度应用
        ZXing::BarcodeFormat::PDF417,        // 55分 - 证件应用
        ZXing::BarcodeFormat::Aztec,         // 50分 - 交通票务
        ZXing::BarcodeFormat::ITF,           // 45分 - 数字应用
        ZXing::BarcodeFormat::Code93,        // 40分 - 扩展应用
        ZXing::BarcodeFormat::Codabar,       // 35分 - 医疗图书
        ZXing::BarcodeFormat::MicroQRCode,   // 30分 - 特殊应用
        ZXing::BarcodeFormat::MaxiCode       // 25分 - 邮政专用
    };
}

int QRCodeRecognizer::getFormatPopularityScore(ZXing::BarcodeFormat format) const
{
    switch (format) {
        case ZXing::BarcodeFormat::QRCode:       return 100;
        case ZXing::BarcodeFormat::EAN13:        return 90;
        case ZXing::BarcodeFormat::EAN8:         return 85;
        case ZXing::BarcodeFormat::UPCA:         return 80;
        case ZXing::BarcodeFormat::UPCE:         return 75;
        case ZXing::BarcodeFormat::Code128:      return 70;
        case ZXing::BarcodeFormat::Code39:       return 65;
        case ZXing::BarcodeFormat::DataMatrix:   return 60;
        case ZXing::BarcodeFormat::PDF417:       return 55;
        case ZXing::BarcodeFormat::Aztec:        return 50;
        case ZXing::BarcodeFormat::ITF:          return 45;
        case ZXing::BarcodeFormat::Code93:       return 40;
        case ZXing::BarcodeFormat::Codabar:      return 35;
        case ZXing::BarcodeFormat::MicroQRCode:  return 30;
        case ZXing::BarcodeFormat::MaxiCode:     return 25;
        default:                                 return 0;
    }
}
