#include "core/QRCodeGenerator.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

// ZXing includes
#include "MultiFormatWriter.h"
#include "BitMatrix.h"

QRCodeGenerator::QRCodeGenerator()
{
}

QRCodeGenerator::~QRCodeGenerator()
{
}

QPixmap QRCodeGenerator::generateQRCode(const GenerationConfig& config)
{
    if (config.text.isEmpty()) {
        m_lastError = "文本不能为空";
        return createErrorImage(config.size, m_lastError);
    }

    try {
        // 使用传统API生成基础条码
        QPixmap baseBarcode = generateWithTraditionalAPI(config);
        
        if (baseBarcode.isNull()) {
            return baseBarcode;
        }
        
        // 如果启用了自定义文本，添加文本
        if (config.enableCustomText && !config.customText.isEmpty()) {
            return addCustomText(baseBarcode, config);
        }
        
        return baseBarcode;
    }
    catch (const std::exception& e) {
        m_lastError = QString("生成二维码失败: %1").arg(e.what());
        qDebug() << m_lastError;
        return createErrorImage(config.size, "生成失败");
    }
}

QPixmap QRCodeGenerator::generateQRCode(const QString& text, const QSize& size)
{
    GenerationConfig config;
    config.text = text;
    config.size = size;
    return generateQRCode(config);
}

QPixmap QRCodeGenerator::generateWithTraditionalAPI(const GenerationConfig& config)
{
    try {
        // 使用ZXing的MultiFormatWriter来生成条码，支持配置中指定的格式
        ZXing::MultiFormatWriter writer(config.format);
        writer.setMargin(config.margin);
        
        // 设置错误纠正级别（仅对支持的格式）
        if (supportsErrorCorrection(config.format)) {
            int eccLevel = convertErrorCorrectionLevel(config.errorCorrection);
            writer.setEccLevel(eccLevel);
        }
        
        // 生成BitMatrix
        auto matrix = writer.encode(config.text.toUtf8().toStdString(), 
                                   config.size.width(), 
                                   config.size.height());
        
        // 转换为QPixmap
        return matrixToPixmap(matrix, config.size);
        
    } catch (const std::exception& e) {
        m_lastError = QString("生成条码失败: %1").arg(e.what());
        qDebug() << m_lastError;
        return createErrorImage(config.size, "生成失败");
    }
}

QPixmap QRCodeGenerator::embedLogo(const QPixmap& qrCode, const QPixmap& logo, int logoSizePercent)
{
    if (qrCode.isNull() || logo.isNull()) {
        m_lastError = "二维码或Logo图像无效";
        return qrCode;
    }

    // 限制logo大小百分比在合理范围内
    logoSizePercent = qBound(5, logoSizePercent, 30);
    
    QPixmap result = qrCode;
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 计算logo尺寸和位置
    int logoSize = qMin(result.width(), result.height()) * logoSizePercent / 100;
    QSize logoTargetSize(logoSize, logoSize);
    
    // 缩放logo
    QPixmap scaledLogo = logo.scaled(logoTargetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // 计算中心位置
    int x = (result.width() - scaledLogo.width()) / 2;
    int y = (result.height() - scaledLogo.height()) / 2;
    
    // 绘制白色背景（提高可读性）
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(x - 5, y - 5, scaledLogo.width() + 10, scaledLogo.height() + 10);
    
    // 绘制logo
    painter.drawPixmap(x, y, scaledLogo);
    
    return result;
}

bool QRCodeGenerator::canEncode(const QString& text) const
{
    // 基本检查
    if (text.isEmpty() || text.length() > 4296) { // QR码最大容量约4296字符
        return false;
    }
    
    // 检查是否包含不支持的字符（这里只做简单检查）
    return true;
}

QString QRCodeGenerator::getLastError() const
{
    return m_lastError;
}

QPixmap QRCodeGenerator::matrixToPixmap(const ZXing::BitMatrix& matrix, const QSize& size)
{
    if (matrix.width() == 0 || matrix.height() == 0) {
        return createErrorImage(size, "无效的矩阵数据");
    }

    // 创建基础图像
    QImage image(matrix.width(), matrix.height(), QImage::Format_RGB32);
    image.fill(Qt::white);
    
    // 填充像素
    for (int y = 0; y < matrix.height(); ++y) {
        for (int x = 0; x < matrix.width(); ++x) {
            if (matrix.get(x, y)) {
                image.setPixel(x, y, qRgb(0, 0, 0));
            }
        }
    }
    
    // 转换为QPixmap并缩放到目标尺寸
    QPixmap pixmap = QPixmap::fromImage(image);
    if (size.isValid() && (pixmap.size() != size)) {
        pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::FastTransformation);
    }
    
    return pixmap;
}

QPixmap QRCodeGenerator::createErrorImage(const QSize& size, const QString& errorMsg)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::lightGray);
    
    QPainter painter(&pixmap);
    painter.setPen(Qt::red);
    painter.setFont(QFont("Arial", 12));
    
    QRect rect = pixmap.rect();
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, errorMsg);
    
    return pixmap;
}

int QRCodeGenerator::convertErrorCorrectionLevel(ErrorCorrectionLevel level)
{
    switch (level) {
        case ErrorCorrectionLevel::Low:      return 0; // L
        case ErrorCorrectionLevel::Medium:   return 1; // M
        case ErrorCorrectionLevel::Quartile: return 2; // Q
        case ErrorCorrectionLevel::High:     return 3; // H
        default:                             return 1; // M
    }
}

bool QRCodeGenerator::supportsErrorCorrection(ZXing::BarcodeFormat format)
{
    // 只有部分格式支持错误纠正级别
    switch (format) {
        case ZXing::BarcodeFormat::QRCode:
        case ZXing::BarcodeFormat::DataMatrix:
        case ZXing::BarcodeFormat::PDF417:
        case ZXing::BarcodeFormat::MicroQRCode:
            return true;
        default:
            return false;
    }
}

QString QRCodeGenerator::getFormatDescription(ZXing::BarcodeFormat format)
{
    switch (format) {
        case ZXing::BarcodeFormat::QRCode:
            return "QR Code - 二维码\n"
                   "• 最流行的二维码格式\n"
                   "• 支持汉字、网址、文本等\n"
                   "• 容量大，纠错能力强\n"
                   "• 适用于各种移动应用";
                   
        case ZXing::BarcodeFormat::MicroQRCode:
            return "Micro QR Code - 微型二维码\n"
                   "• QR码的小型版本\n"
                   "• 适用于空间受限的场景\n"
                   "• 数据容量较小\n"
                   "• 主要用于工业标识";
                   
        case ZXing::BarcodeFormat::DataMatrix:
            return "Data Matrix - 数据矩阵\n"
                   "• 高密度二维码\n"
                   "• 在小空间内存储大量数据\n"
                   "• 常用于医疗和制造业\n"
                   "• 支持错误纠正";
                   
        case ZXing::BarcodeFormat::PDF417:
            return "PDF417 - 便携式数据文件417\n"
                   "• 大容量二维条码\n"
                   "• 可存储文档、照片等\n"
                   "• 广泛用于身份证件\n"
                   "• 支持多级错误纠正";
                   
        case ZXing::BarcodeFormat::Code128:
            return "Code 128 - 一维条码\n"
                   "• 支持完整ASCII字符集\n"
                   "• 数据密度高\n"
                   "• 广泛用于物流运输\n"
                   "• 可变长度编码";
                   
        case ZXing::BarcodeFormat::Code39:
            return "Code 39 - 三九码\n"
                   "• 经典一维条码\n"
                   "• 支持数字和大写字母\n"
                   "• 工业应用广泛\n"
                   "• 结构简单可靠";
                   
        case ZXing::BarcodeFormat::Code93:
            return "Code 93 - 九三码\n"
                   "• Code 39的增强版\n"
                   "• 更高的数据密度\n"
                   "• 支持扩展字符集\n"
                   "• 包含校验机制";
                   
        case ZXing::BarcodeFormat::Codabar:
            return "Codabar - 库德巴码\n"
                   "• 专用于数字应用\n"
                   "• 医疗和图书馆常用\n"
                   "• 结构简单\n"
                   "• 易于手工识别";
                   
        case ZXing::BarcodeFormat::EAN8:
            return "EAN-8 - 欧洲商品码(8位)\n"
                   "• 国际标准商品码\n"
                   "• 用于小包装商品\n"
                   "• 8位数字编码\n"
                   "• 包含校验位";
                   
        case ZXing::BarcodeFormat::EAN13:
            return "EAN-13 - 欧洲商品码(13位)\n"
                   "• 全球标准商品条码\n"
                   "• 13位数字编码\n"
                   "• 超市扫描必备\n"
                   "• 包含国家和厂商信息";
                   
        case ZXing::BarcodeFormat::UPCA:
            return "UPC-A - 美国通用商品码\n"
                   "• 北美标准商品码\n"
                   "• 12位数字编码\n"
                   "• 零售业广泛使用\n"
                   "• 与EAN-13兼容";
                   
        case ZXing::BarcodeFormat::UPCE:
            return "UPC-E - 美国商品码(缩短版)\n"
                   "• UPC-A的压缩版本\n"
                   "• 用于小包装商品\n"
                   "• 6位数字显示\n"
                   "• 节省标签空间";
                   
        case ZXing::BarcodeFormat::ITF:
            return "ITF - 交错25码\n"
                   "• 仅支持偶数位数字\n"
                   "• 高密度编码\n"
                   "• 物流行业常用\n"
                   "• 适合工业环境";
                   
        case ZXing::BarcodeFormat::Aztec:
            return "Aztec - 阿兹特克码\n"
                   "• 高效的二维码\n"
                   "• 无需定位标记\n"
                   "• 适应性强\n"
                   "• 用于交通票务";
                   
        case ZXing::BarcodeFormat::MaxiCode:
            return "MaxiCode - 最大码\n"
                   "• UPS专用格式\n"
                   "• 用于邮政分拣\n"
                   "• 固定尺寸\n"
                   "• 高速识别";
                   
        default:
            return "未知格式";
    }
}

QPixmap QRCodeGenerator::addCustomText(const QPixmap& barcode, const GenerationConfig& config)
{
    if (barcode.isNull() || config.customText.isEmpty()) {
        return barcode;
    }
    
    // 设置字体
    QFont font("Arial", config.textSize, QFont::Bold);
    QFontMetrics fontMetrics(font);
    
    // 计算文本尺寸 - 使用单行文本
    QString text = config.customText.trimmed(); // 去除首尾空格
    // 移除换行符，确保单行显示
    text = text.replace('\n', ' ').replace('\r', ' ');
    
    QRect textRect = fontMetrics.boundingRect(text);
    int textWidth = textRect.width();
    int textHeight = fontMetrics.height(); // 使用字体高度而不是边界高度
    
    // 计算最终图像尺寸
    QSize finalSize;
    QPoint barcodePos;
    QPoint textPos;
    
    const int padding = 15; // 文本与条码间的间距
    const int margin = 10;  // 边距
    
    switch (config.textPosition) {
        case TextPosition::Bottom:
            finalSize = QSize(qMax(barcode.width(), textWidth + margin * 2), 
                             barcode.height() + textHeight + padding + margin);
            barcodePos = QPoint((finalSize.width() - barcode.width()) / 2, margin / 2);
            textPos = QPoint((finalSize.width() - textWidth) / 2, 
                           barcode.height() + padding + fontMetrics.ascent());
            break;
            
        case TextPosition::Top:
            finalSize = QSize(qMax(barcode.width(), textWidth + margin * 2), 
                             barcode.height() + textHeight + padding + margin);
            barcodePos = QPoint((finalSize.width() - barcode.width()) / 2, 
                               textHeight + padding);
            textPos = QPoint((finalSize.width() - textWidth) / 2, fontMetrics.ascent());
            break;
            
        case TextPosition::Left:
            finalSize = QSize(barcode.width() + textWidth + padding + margin * 2, 
                             qMax(barcode.height(), textHeight) + margin);
            barcodePos = QPoint(textWidth + padding + margin, 
                               (finalSize.height() - barcode.height()) / 2);
            textPos = QPoint(margin, 
                           (finalSize.height() + fontMetrics.ascent()) / 2);
            break;
            
        case TextPosition::Right:
            finalSize = QSize(barcode.width() + textWidth + padding + margin * 2, 
                             qMax(barcode.height(), textHeight) + margin);
            barcodePos = QPoint(margin, (finalSize.height() - barcode.height()) / 2);
            textPos = QPoint(barcode.width() + padding + margin, 
                           (finalSize.height() + fontMetrics.ascent()) / 2);
            break;
    }
    
    // 创建最终图像
    QPixmap result(finalSize);
    result.fill(Qt::white);
    
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    
    // 绘制条码
    painter.drawPixmap(barcodePos, barcode);
    
    // 绘制文本 - 使用单行绘制
    painter.setFont(font);
    painter.setPen(config.textColor);
    painter.drawText(textPos, text);
    
    painter.end(); // 确保绘制完成
    
    return result;
}
