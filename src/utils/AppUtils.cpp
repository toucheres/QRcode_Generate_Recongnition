#include "utils/AppUtils.h"
#include <QRegularExpression>
#include <QImageReader>
#include <QDir>

bool AppUtils::isValidUrl(const QString& url)
{
    if (url.isEmpty()) {
        return false;
    }

    QUrl qurl(url);
    return qurl.isValid() && !qurl.scheme().isEmpty() && !qurl.host().isEmpty();
}

bool AppUtils::isValidFilePath(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }

    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile() && fileInfo.isReadable();
}

QString AppUtils::getImageFileFilter()
{
    QStringList supportedFormats;
    
    // 获取Qt支持的图像格式
    const auto formats = QImageReader::supportedImageFormats();
    for (const auto& format : formats) {
        supportedFormats << QString("*.%1").arg(QString::fromUtf8(format).toLower());
    }
    
    return QString("图像文件 (%1);;所有文件 (*.*)").arg(supportedFormats.join(" "));
}

bool AppUtils::isImageFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }
    
    QString extension = fileInfo.suffix().toLower();
    
    // 常见的图像格式
    QStringList imageExtensions = {
        "jpg", "jpeg", "png", "bmp", "gif", "tiff", "tif", 
        "svg", "webp", "ico", "pbm", "pgm", "ppm", "xbm", "xpm"
    };
    
    return imageExtensions.contains(extension);
}

QString AppUtils::getSaveImageFileFilter()
{
    return "PNG 图片 (*.png);;JPEG 图片 (*.jpg *.jpeg);;BMP 图片 (*.bmp);;SVG 矢量图 (*.svg);;所有文件 (*.*)";
}

QString AppUtils::formatFileSize(qint64 bytes)
{
    const qint64 kb = 1024;
    const qint64 mb = kb * 1024;
    const qint64 gb = mb * 1024;

    if (bytes >= gb) {
        return QString::number(bytes / (double)gb, 'f', 2) + " GB";
    } else if (bytes >= mb) {
        return QString::number(bytes / (double)mb, 'f', 2) + " MB";
    } else if (bytes >= kb) {
        return QString::number(bytes / (double)kb, 'f', 2) + " KB";
    } else {
        return QString::number(bytes) + " B";
    }
}

QString AppUtils::getFileExtension(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}

bool AppUtils::containsChinese(const QString& text)
{
    // 检查是否包含中文字符（Unicode范围）
    QRegularExpression chineseRegex("[\\u4e00-\\u9fa5]");
    return chineseRegex.match(text).hasMatch();
}

QString AppUtils::truncateString(const QString& text, int maxLength, const QString& ellipsis)
{
    if (text.length() <= maxLength) {
        return text;
    }
    
    return text.left(maxLength - ellipsis.length()) + ellipsis;
}

QString AppUtils::generateUniqueFileName(const QString& basePath, const QString& fileName)
{
    QFileInfo fileInfo(fileName);
    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    
    QString fullPath = QDir(basePath).filePath(fileName);
    
    if (!QFile::exists(fullPath)) {
        return fullPath;
    }
    
    int counter = 1;
    while (true) {
        QString newFileName = QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension);
        QString newFullPath = QDir(basePath).filePath(newFileName);
        
        if (!QFile::exists(newFullPath)) {
            return newFullPath;
        }
        
        counter++;
    }
}

bool AppUtils::isEmail(const QString& text)
{
    // 简单的邮箱验证正则表达式
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return emailRegex.match(text).hasMatch();
}

bool AppUtils::isPhoneNumber(const QString& text)
{
    // 检查是否为中国手机号或固定电话
    QRegularExpression phoneRegex("^(\\+86|86)?\\s*1[3-9]\\d{9}$|^(\\+86|86)?\\s*0\\d{2,3}\\s*\\d{7,8}$");
    return phoneRegex.match(text.simplified()).hasMatch();
}
