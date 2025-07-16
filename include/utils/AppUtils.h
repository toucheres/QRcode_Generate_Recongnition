#pragma once

#include <QString>
#include <QUrl>
#include <QFileInfo>

/**
 * @class AppUtils
 * @brief 应用程序工具类，提供通用的工具方法
 */
class AppUtils
{
public:
    /**
     * @brief 验证URL是否有效
     * @param url URL字符串
     * @return 如果URL有效返回true
     */
    static bool isValidUrl(const QString& url);

    /**
     * @brief 验证文件路径是否有效
     * @param filePath 文件路径
     * @return 如果文件存在且可读返回true
     */
    static bool isValidFilePath(const QString& filePath);

    /**
     * @brief 获取支持的图像格式文件过滤器
     * @return 文件对话框过滤器字符串
     */
    static QString getImageFileFilter();

    /**
     * @brief 检查文件是否为支持的图像格式
     * @param filePath 文件路径
     * @return 如果是支持的图像格式返回true
     */
    static bool isImageFile(const QString& filePath);

    /**
     * @brief 获取保存图像文件的过滤器
     * @return 保存文件对话框过滤器字符串
     */
    static QString getSaveImageFileFilter();

    /**
     * @brief 格式化文件大小
     * @param bytes 字节数
     * @return 格式化后的大小字符串（如：1.5 MB）
     */
    static QString formatFileSize(qint64 bytes);

    /**
     * @brief 获取文件扩展名
     * @param filePath 文件路径
     * @return 文件扩展名（小写，不含点）
     */
    static QString getFileExtension(const QString& filePath);

    /**
     * @brief 检查字符串是否包含中文字符
     * @param text 待检查的字符串
     * @return 如果包含中文字符返回true
     */
    static bool containsChinese(const QString& text);

    /**
     * @brief 限制字符串长度并添加省略号
     * @param text 原始字符串
     * @param maxLength 最大长度
     * @param ellipsis 省略号字符串
     * @return 处理后的字符串
     */
    static QString truncateString(const QString& text, int maxLength, const QString& ellipsis = "...");

    /**
     * @brief 生成唯一的文件名（避免重复）
     * @param basePath 基础路径
     * @param fileName 原始文件名
     * @return 唯一的文件路径
     */
    static QString generateUniqueFileName(const QString& basePath, const QString& fileName);

    /**
     * @brief 检查文本是否为邮箱地址
     * @param text 待检查的文本
     * @return 如果是有效邮箱返回true
     */
    static bool isEmail(const QString& text);

    /**
     * @brief 检查文本是否为电话号码
     * @param text 待检查的文本
     * @return 如果是有效电话号码返回true
     */
    static bool isPhoneNumber(const QString& text);

private:
    AppUtils() = delete; // 工具类不允许实例化
};
