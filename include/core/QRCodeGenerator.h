#pragma once

#include <QString>
#include <QPixmap>
#include <QSize>
#include <string>

// ZXing includes
#include "BarcodeFormat.h"
#ifdef ZXING_EXPERIMENTAL_API
    #include "WriteBarcode.h"
#else
    #include "MultiFormatWriter.h"
    #include "BitMatrix.h"
#endif

/**
 * @class QRCodeGenerator
 * @brief 二维码生成器类，负责生成不同格式的二维码
 */
class QRCodeGenerator
{
public:
    /**
     * @brief 错误纠正级别枚举
     */
    enum class ErrorCorrectionLevel {
        Low,      // L: ~7%
        Medium,   // M: ~15%
        Quartile, // Q: ~25%
        High      // H: ~30%
    };

    /**
     * @brief 文本位置枚举
     */
    enum class TextPosition {
        Bottom,    // 底部
        Top,       // 顶部
        Left,      // 左侧
        Right      // 右侧
    };

    /**
     * @brief 二维码生成配置结构
     */
    struct GenerationConfig {
        QString text;                           // 要编码的文本
        QSize size;                            // 生成尺寸
        ErrorCorrectionLevel errorCorrection;  // 错误纠正级别
        int margin;                            // 边距
        ZXing::BarcodeFormat format;           // 条码格式
        
        // 自定义文本配置
        bool enableCustomText;                  // 是否启用自定义文本
        QString customText;                     // 自定义文本内容
        TextPosition textPosition;              // 文本位置
        int textSize;                          // 文本字体大小
        QColor textColor;                      // 文本颜色
        
        GenerationConfig() 
            : size(300, 300)
            , errorCorrection(ErrorCorrectionLevel::Medium)
            , margin(10)
            , format(ZXing::BarcodeFormat::QRCode)
            , enableCustomText(false)
            , textPosition(TextPosition::Bottom)
            , textSize(12)
            , textColor(Qt::black)
        {}
        
        GenerationConfig(const QString& text) 
            : text(text)
            , size(300, 300)
            , errorCorrection(ErrorCorrectionLevel::Medium)
            , margin(10)
            , format(ZXing::BarcodeFormat::QRCode)
            , enableCustomText(false)
            , textPosition(TextPosition::Bottom)
            , textSize(12)
            , textColor(Qt::black)
        {}
        
        GenerationConfig(const GenerationConfig&) = default;
        GenerationConfig& operator=(const GenerationConfig&) = default;
    };

public:
    QRCodeGenerator();
    ~QRCodeGenerator();

    /**
     * @brief 生成二维码
     * @param config 生成配置
     * @return 生成的二维码图像，失败时返回空的QPixmap
     */
    QPixmap generateQRCode(const GenerationConfig& config);

    /**
     * @brief 简化的二维码生成方法
     * @param text 要编码的文本
     * @param size 生成尺寸，默认300x300
     * @return 生成的二维码图像
     */
    QPixmap generateQRCode(const QString& text, const QSize& size = QSize(300, 300));

    /**
     * @brief 嵌入logo到二维码中
     * @param qrCode 原始二维码
     * @param logo logo图像
     * @param logoSizePercent logo占二维码的百分比（5-30之间）
     * @return 嵌入logo后的二维码
     */
    QPixmap embedLogo(const QPixmap& qrCode, const QPixmap& logo, int logoSizePercent = 20);

    /**
     * @brief 为条码添加自定义文本
     * @param barcode 原始条码图像
     * @param config 生成配置（包含文本信息）
     * @return 添加文本后的图像
     */
    QPixmap addCustomText(const QPixmap& barcode, const GenerationConfig& config);

    /**
     * @brief 检查文本是否可以安全编码
     * @param text 待检查的文本
     * @return 如果可以编码返回true
     */
    bool canEncode(const QString& text) const;

    /**
     * @brief 获取错误信息
     * @return 最后一次操作的错误信息
     */
    QString getLastError() const;

    /**
     * @brief 获取条码格式的详细说明
     * @param format 条码格式
     * @return 格式说明字符串
     */
    QString getFormatDescription(ZXing::BarcodeFormat format);
    /**
     * @brief 检查指定格式是否支持错误纠正
     * @param format 条码格式
     * @return 是否支持错误纠正
     */
    bool supportsErrorCorrection(ZXing::BarcodeFormat format);

  private:
    /**
     * @brief 将ZXing的BitMatrix转换为QPixmap
     * @param matrix ZXing的BitMatrix
     * @param size 目标尺寸
     * @return 转换后的QPixmap
     */
    QPixmap matrixToPixmap(const ZXing::BitMatrix& matrix, const QSize& size);

    /**
     * @brief 创建错误提示图像
     * @param size 图像尺寸
     * @param errorMsg 错误信息
     * @return 错误提示图像
     */
    QPixmap createErrorImage(const QSize& size, const QString& errorMsg = "生成失败");

    /**
     * @brief 使用传统API生成二维码
     * @param config 生成配置
     * @return 生成的二维码
     */
    QPixmap generateWithTraditionalAPI(const GenerationConfig& config);

    /**
     * @brief 转换错误纠正级别
     * @param level 内部错误纠正级别
     * @return ZXing错误纠正级别
     */
    int convertErrorCorrectionLevel(ErrorCorrectionLevel level);

    /**
     * @brief 验证格式和文本的兼容性
     * @param format 条码格式
     * @param text 文本内容
     * @return 错误信息，空字符串表示验证通过
     */
    QString validateFormatAndText(ZXing::BarcodeFormat format, const QString& text);

    /**
     * @brief 检查指定格式是否支持字符编码设置
     * @param format 条码格式
     * @return 是否支持编码设置
     */
    bool supportsEncoding(ZXing::BarcodeFormat format);

    /**
     * @brief 根据格式准备文本编码
     * @param format 条码格式
     * @param text 原始文本
     * @return 适合该格式的编码字符串
     */
    std::string prepareTextForFormat(ZXing::BarcodeFormat format, const QString& text);

private:
    QString m_lastError;
};
