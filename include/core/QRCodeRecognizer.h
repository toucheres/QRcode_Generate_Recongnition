#pragma once

#include <QString>
#include <QImage>
#include <QPixmap>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>

// ZXing includes
#include "ReadBarcode.h"
#include "Quadrilateral.h"

/**
 * @class QRCodeRecognizer
 * @brief 二维码识别器类，负责从图像中识别二维码
 */
class QRCodeRecognizer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 识别结果结构
     */
    struct RecognitionResult {
        QString text;                       // 识别到的文本
        bool isValid;                       // 是否识别成功
        ZXing::QuadrilateralI position;    // 二维码的四个角点位置
        QString format;                     // 条码格式
        double confidence;                  // 置信度（0-1）
        
        RecognitionResult() 
            : isValid(false)
            , confidence(0.0)
        {}
        
        RecognitionResult(const QString& text, const ZXing::QuadrilateralI& pos, 
                         const QString& format, double conf = 1.0)
            : text(text), isValid(true), position(pos), format(format), confidence(conf) {}
        
        RecognitionResult(const RecognitionResult&) = default;
        RecognitionResult& operator=(const RecognitionResult&) = default;
    };

    /**
     * @brief 识别配置结构
     */
    struct RecognitionConfig {
        bool tryHarder;             // 是否使用更严格的识别
        bool tryRotate;             // 是否尝试旋转图像
        bool fastMode;              // 快速模式（降低精度提高速度）
        int maxSymbols;             // 最大识别符号数量
        
        RecognitionConfig() 
            : tryHarder(false)
            , tryRotate(true)
            , fastMode(false)
            , maxSymbols(1)
        {}
        
        RecognitionConfig(const RecognitionConfig&) = default;
        RecognitionConfig& operator=(const RecognitionConfig&) = default;
    };

public:
    explicit QRCodeRecognizer(QObject* parent = nullptr);
    ~QRCodeRecognizer();

    /**
     * @brief 同步识别二维码（单张图片）
     * @param image 待识别的图像
     * @param config 识别配置
     * @return 识别结果
     */
    RecognitionResult recognizeSync(const QImage& image, const RecognitionConfig& config = {});

    /**
     * @brief 识别多种格式的条码，按流行度排序返回
     * @param image 待识别的图像
     * @param config 识别配置
     * @return 按流行度排序的识别结果列表
     */
    QList<RecognitionResult> recognizeMultiFormat(const QImage& image, const RecognitionConfig& config = {});

    /**
     * @brief 获取支持的条码格式列表，按流行度排序
     * @return 格式列表（显示名称）
     */
    QStringList getSupportedFormats() const;

    /**
     * @brief 异步识别二维码
     * @param image 待识别的图像
     * @param requestId 请求ID
     * @param config 识别配置
     */
    void recognizeAsync(const QImage& image, int requestId, const RecognitionConfig& config = {});

    /**
     * @brief 从QPixmap识别二维码
     * @param pixmap 待识别的QPixmap
     * @param config 识别配置
     * @return 识别结果
     */
    RecognitionResult recognizeFromPixmap(const QPixmap& pixmap, const RecognitionConfig& config = {});

    /**
     * @brief 在图像上绘制识别结果轮廓
     * @param originalImage 原始图像
     * @param result 识别结果
     * @return 绘制轮廓后的图像
     */
    QPixmap drawContour(const QPixmap& originalImage, const RecognitionResult& result);

    /**
     * @brief 获取错误信息
     * @return 最后一次操作的错误信息
     */
    QString getLastError() const;

    /**
     * @brief 设置识别配置
     * @param config 识别配置
     */
    void setConfig(const RecognitionConfig& config);

signals:
    /**
     * @brief 异步识别完成信号
     * @param result 识别结果
     * @param requestId 请求ID
     */
    void recognitionCompleted(const QRCodeRecognizer::RecognitionResult& result, int requestId);

    /**
     * @brief 异步识别失败信号
     * @param error 错误信息
     * @param requestId 请求ID
     */
    void recognitionFailed(const QString& error, int requestId);

private slots:
    void processRecognitionQueue();

private:
    /**
     * @brief 执行实际的识别操作
     * @param image 待识别的图像
     * @param config 识别配置
     * @return 识别结果
     */
    RecognitionResult performRecognition(const QImage& image, const RecognitionConfig& config);

    /**
     * @brief 转换ZXing的ReaderOptions
     * @param config 内部配置
     * @return ZXing的ReaderOptions
     */
    ZXing::ReaderOptions convertConfig(const RecognitionConfig& config);

    /**
     * @brief 预处理图像（调整大小、格式转换等）
     * @param image 原始图像
     * @return 预处理后的图像
     */
    QImage preprocessImage(const QImage& image);

    /**
     * @brief 获取按流行度排序的条码格式
     * @return 格式枚举列表
     */
    QList<ZXing::BarcodeFormat> getFormatsOrderedByPopularity() const;

    /**
     * @brief 计算识别结果的流行度得分
     * @param format 条码格式
     * @return 流行度得分（越高越流行）
     */
    int getFormatPopularityScore(ZXing::BarcodeFormat format) const;

private:
    struct AsyncRequest {
        QImage image;
        int requestId;
        RecognitionConfig config;
        qint64 timestamp;
    };

    RecognitionConfig m_defaultConfig;
    QString m_lastError;
    
    // 异步处理相关
    QQueue<AsyncRequest> m_requestQueue;
    QTimer* m_processTimer;
    QMutex m_queueMutex;
    bool m_processing{false};
    
    // 常量
    static const int MAX_QUEUE_SIZE = 5;
    static const int PROCESSING_INTERVAL = 100; // ms
};
