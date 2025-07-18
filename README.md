# QR码生成识别器 v3.0

一个基于Qt6和ZXing-C++库开发的全功能QR码生成和识别应用程序，采用现代化模块化架构设计。

## 🎯 功能特性

### 二维码生成
- ✅ 支持多种格式（QR Code, PDF417, Code128, Code39, Data Matrix等）
- ✅ 四级错误纠正级别支持（L, M, Q, H）
- ✅ 自定义尺寸设置（100px - 1000px）
- ✅ Logo嵌入功能，可调节Logo大小和透明度
- ✅ 完整的UTF-8字符支持（中文、日文、韩文、阿拉伯文等）
- ✅ PDF417格式优化（自动ASCII转换，防止中文编码崩溃）
- ✅ 多种格式导出（PNG, JPEG, BMP, SVG）
- ✅ 批量生成和预览功能
- ✅ 实时输入验证和格式兼容性检查

### 二维码识别
- ✅ 图片文件识别（支持多种格式）
- ✅ 摄像头实时识别
- ✅ 批量识别处理
- ✅ 识别结果轮廓显示
- ✅ 识别历史记录
- ✅ **自动打开URL功能**（识别到网址时可自动在浏览器中打开）
- ✅ 结果复制和分享

### 智能设置系统
- ✅ **自动打开URL设置**（可在设置中开启/关闭）
- ✅ 主题切换（明暗主题自动检测）
- ✅ 语言本地化支持
- ✅ 个性化偏好设置持久化存储
- ✅ 一键恢复默认设置

### 用户界面
- ✅ 现代化Material Design风格界面
- ✅ 标签页式多功能布局
- ✅ **智能响应式设计**（优化小窗口显示，支持滚动）
- ✅ 支持键盘快捷键
- ✅ 状态栏实时反馈
- ✅ **紧凑型UI布局**（生成器界面支持垂直滚动，适配小屏幕）
- ✅ 暗色/明色主题自动适配

## 🏗️ 技术架构

### 核心技术栈
- **Qt6** - 跨平台GUI框架
- **ZXing-C++** - 高性能条码处理库
- **CMake 3.16+** - 现代化构建系统
- **C++17** - 现代C++标准

### 模块化架构

```
src/
├── main.cpp               # 程序入口
├── mainwindow.cpp/h       # 主窗口控制器
└── widgets/               # UI组件模块
    ├── BaseWidget.*       # UI基类（主题检测）
    ├── GeneratorWidget.*  # 生成器界面（滚动支持）
    ├── RecognizerWidget.* # 识别器界面（URL自动打开）
    ├── CameraWidget.*     # 摄像头识别（实时识别+URL打开）
    └── SettingsDialog.*   # 设置对话框
├── core/                  # 核心业务逻辑
    ├── QRCodeGenerator.*  # 二维码生成器（多格式支持）
    └── QRCodeRecognizer.* # 二维码识别器
├── utils/                 # 工具类
    └── AppSettings.*      # 应用设置管理（URL自动打开）

include/                   # 头文件
forms/                     # UI资源文件
docs/                      # 文档
3rd/zxing/                # ZXing-C++第三方库
```

### 新增核心功能模块

#### AppSettings 设置管理系统
```cpp
class AppSettings {
public:
    static bool autoOpenUrls();              // 获取URL自动打开设置
    static void setAutoOpenUrls(bool enable); // 设置URL自动打开
    static bool tryOpenUrl(const QString& url); // 尝试打开URL
    // ... 其他设置项
};
```

#### 智能UI布局系统
- **GeneratorWidget**: 采用QScrollArea实现垂直滚动，完美适配小窗口
- **BaseWidget**: 提供主题检测和通用UI基础功能
- **响应式布局**: 自动调整组件大小和布局以适应不同屏幕

include/                   # 头文件（对应src结构）
forms/                     # UI资源文件
docs/                      # 文档
tests/                     # 测试用例（预留）
3rd/zxing/                # ZXing-C++第三方库
```

## 🔧 系统要求

### 开发环境
- **CMake**: 3.16 或更高版本
- **编译器**: 支持C++17的编译器
  - Windows: Visual Studio 2019+ 或 MinGW-w64
  - Linux: GCC 7+ 或 Clang 5+
  - macOS: Xcode 10+

### 运行时依赖
- **Qt6**: Core, Widgets, MultimediaWidgets, Multimedia, Core5Compat, Svg
- **操作系统**: Windows 10+, Ubuntu 18.04+, macOS 10.14+
- **摄像头**: 用于实时识别功能（可选）

## 📦 安装和构建

### 1. 克隆项目

```bash
git clone https://github.com/toucheres/QRcode_Generate_Recongnition
cd QRcode_Generate_Recongnition
```

### 2. 设置环境变量

确保Qt6安装路径设置正确：

#### Windows
```cmd
set QTDIR=C:\Qt\6.x.x\msvc2019_64
set PATH=%QTDIR%\bin;%PATH%
```

#### Linux/macOS
```bash
export QTDIR=/path/to/qt6
export PATH=$QTDIR/bin:$PATH
```

### 3. 构建项目

#### 使用CMake命令行
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### 使用Qt Creator
1. 打开Qt Creator
2. 选择 "Open Project"
3. 选择项目根目录下的 `CMakeLists.txt`
4. 配置构建套件
5. 构建并运行

### 4. 运行程序

构建完成后，可执行文件位于：
- Windows: `build/QRcode_Generator_Recongniser.exe`
- Linux/macOS: `build/QRcode_Generator_Recongniser`

## 📖 使用说明

### 🎨 二维码生成

1. **启动应用程序并切换到"生成"标签页**
2. **选择条码格式**
   - QR Code: 最常用的二维码格式
   - PDF417: 适合大容量数据（自动ASCII转换，支持中文）
   - Code128/Code39: 一维条码格式
   - Data Matrix: 紧凑型二维码
3. **输入内容**
   - 支持所有UTF-8字符（中文、英文、数字、特殊符号）
   - 实时验证输入内容与格式兼容性
   - PDF417会自动将中文转换为安全的ASCII编码
4. **调整设置**
   - 错误纠正级别（L/M/Q/H）
   - 尺寸设置（100-1000像素）
   - Logo嵌入（可选，支持透明度调节）
5. **生成和保存**
   - 点击"生成"按钮或按回车键
   - 支持PNG、JPEG、BMP、SVG格式导出

### 🔍 二维码识别

#### 图片识别
1. **切换到"识别"标签页**
2. **选择识别方式**
   - 文件识别：选择本地图片文件
   - 批量识别：一次处理多个文件
3. **查看结果**
   - 识别的内容会显示在结果区域
   - 支持结果复制和分享
   - **自动URL打开**：如果识别到网址，会询问是否在浏览器中打开

#### 摄像头实时识别
1. **切换到"摄像头"标签页**
2. **启动摄像头**
   - 自动检测可用摄像头设备
   - 实时视频预览
3. **实时识别**
   - 将QR码对准摄像头
   - 识别成功后会显示结果
   - **智能URL处理**：识别到网址时根据设置自动打开

### ⚙️ 设置配置

1. **打开设置对话框**
   - 通过菜单 "设置" -> "偏好设置"
2. **配置选项**
   - **自动打开URL**：识别到网址时是否自动在浏览器中打开（默认开启）
   - 界面主题：自动检测系统主题
   - 语言设置：多语言支持
3. **保存设置**
   - 所有设置自动保存到本地配置文件
   - 支持一键恢复默认设置

## 🛠️ 开发说明

### 核心架构设计

#### 1. BaseWidget 基础组件
```cpp
class BaseWidget : public QWidget {
protected:
    bool isDarkTheme() const;  // 主题检测
    virtual void setupUI() = 0; // 纯虚函数，子类实现具体UI
};
```

#### 2. 智能设置系统
```cpp
class AppSettings {
public:
    // URL自动打开功能
    static bool autoOpenUrls();                    // 获取设置（默认true）
    static void setAutoOpenUrls(bool enable);      // 设置保存
    static bool tryOpenUrl(const QString& url);    // 安全URL打开
    
    // 设置持久化
    static void saveSettings();
    static void loadSettings();
    static void resetToDefaults();
};
```

#### 3. 响应式UI设计
```cpp
// GeneratorWidget - 滚动支持
void GeneratorWidget::setupUI() {
    auto scrollArea = new QScrollArea(this);
    auto settingsWidget = new QWidget();
    
    // 设置最小尺寸以支持小窗口
    scrollArea->setMinimumHeight(400);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(settingsWidget);
}
```

### 关键技术特性

#### 1. PDF417中文处理
```cpp
QString QRCodeGenerator::prepareTextForFormat(const QString& text, BarcodeFormat format) {
    if (format == BarcodeFormat::PDF417) {
        // PDF417需要ASCII编码，防止中文崩溃
        QByteArray asciiData = text.toLatin1();
        return QString::fromLatin1(asciiData);
    }
    return text;  // 其他格式保持UTF-8
}
```

#### 2. 智能URL识别和打开
```cpp
void RecognizerWidget::handleRecognitionResult(const QString& result) {
    if (AppSettings::autoOpenUrls() && isUrl(result)) {
        int ret = QMessageBox::question(this, "打开网址", 
            QString("检测到网址：%1\n\n是否在浏览器中打开？").arg(result));
        
        if (ret == QMessageBox::Yes) {
            AppSettings::tryOpenUrl(result);
        }
    }
}
```

#### 3. 错误处理和输入验证
```cpp
bool QRCodeGenerator::validateFormatAndText(BarcodeFormat format, const QString& text) {
    switch (format) {
        case BarcodeFormat::PDF417:
            return text.length() <= 1800;  // PDF417容量限制
        case BarcodeFormat::QRCode:
            return text.toUtf8().length() <= 2953;  // QR码字节限制
        default:
            return !text.isEmpty();
    }
}
```

### UI/UX 设计原则

1. **响应式布局**：所有界面组件都支持动态调整大小
2. **滚动支持**：复杂设置界面采用滚动区域，确保小屏幕可用性
3. **主题适配**：自动检测系统主题，提供一致的视觉体验
4. **用户反馈**：所有操作都有明确的状态提示和错误信息
5. **无障碍性**：键盘快捷键支持，友好的错误提示

### 调试和测试

```cpp
// 开发时启用详细调试输出
qDebug() << "Input text:" << text;
qDebug() << "UTF-8 bytes:" << text.toUtf8().toHex();
qDebug() << "Format compatibility:" << validateFormatAndText(format, text);
```
## 📋 版本历史

### v3.0.0 (当前版本) - 2024年
- ✅ **重大更新：完整的识别功能**
  - 图片文件识别
  - 摄像头实时识别
  - 批量识别处理
- ✅ **智能URL处理系统**
  - 自动识别网址并提供打开选项
  - 可配置的自动打开设置
  - 安全的URL验证和处理
- ✅ **UI/UX 重大改进**
  - 响应式设计，完美适配小窗口
  - 生成器界面支持垂直滚动
  - 暗色/明色主题自动适配
- ✅ **多格式条码支持**
  - PDF417格式优化（中文安全处理）
  - Code128, Code39, Data Matrix支持
  - 智能格式兼容性检查
- ✅ **设置系统完善**
  - 持久化配置存储
  - 一键恢复默认设置
  - 用户偏好设置界面

### v2.0.0
- ✅ 模块化架构重构
- ✅ Qt6移植完成
- ✅ 基础识别功能框架

### v1.0.0
- ✅ 基本QR码生成功能
- ✅ UTF-8字符支持
- ✅ 图形用户界面
- ✅ 错误处理和用户反馈

### 🚀 未来计划
- 🔄 云端识别服务集成
- 🔄 批量二维码生成模板
- 🔄 二维码样式自定义（颜色、形状、边框）
- 🔄 识别结果数据库存储
- 🔄 网络分享功能
- 🔄 插件系统架构
- 🔄 多语言本地化完善

## ❓ 常见问题

### 安装和构建

**Q: 编译时找不到Qt6**
**A**: 确保设置了正确的QTDIR环境变量，并且Qt6已正确安装。检查PATH中包含Qt6的bin目录。

**Q: CMake配置失败**
**A**: 确保CMake版本>=3.16，并且已安装所需的Qt6组件（Core, Widgets, MultimediaWidgets等）。

### 功能使用

**Q: PDF417格式生成时崩溃**
**A**: v3.0已修复此问题。PDF417现在会自动将中文字符转换为安全的ASCII编码。

**Q: 中文字符显示为乱码**
**A**: 确保源代码文件以UTF-8编码保存。对于PDF417格式，系统会自动处理中文字符。

**Q: 生成的QR码无法扫描**
**A**: 检查输入文本长度是否超出格式限制。不同格式有不同的容量限制：
- QR Code: 约2953字节（UTF-8）
- PDF417: 约1800字符

**Q: 摄像头无法启动**
**A**: 检查摄像头权限设置，确保系统允许应用访问摄像头设备。

**Q: 识别的网址没有自动打开**
**A**: 检查设置中的"自动打开URL"选项是否开启。可以通过菜单"设置"→"偏好设置"进行配置。

### 性能优化

**Q: 大尺寸二维码生成速度慢**
**A**: 这是正常现象。建议根据实际需要选择合适的尺寸，通常500px已足够日常使用。

**Q: 批量识别处理缓慢**
**A**: 批量识别会依次处理每个文件。对于大量文件，建议分批处理或选择较小的图片文件。

## 🤝 贡献指南

我们欢迎所有形式的贡献！请遵循以下步骤：

### 提交代码
1. Fork 项目到你的账户
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 确保代码符合项目规范：
   - 使用统一的代码风格（建议使用clang-format）
   - 添加必要的注释和文档
   - 确保所有功能都有相应的错误处理
4. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
5. 推送到分支 (`git push origin feature/AmazingFeature`)
6. 开启 Pull Request

### 报告问题
- 使用 GitHub Issues 报告 bug
- 提供详细的重现步骤
- 包含系统信息（操作系统、Qt版本、编译器版本）
- 如果可能，请提供相关的日志输出

### 建议功能
- 通过 Issues 提出功能请求
- 详细描述功能的使用场景和预期行为
- 如果有界面相关的建议，可以提供设计草图

### 代码规范
- **C++**: 遵循 Google C++ Style Guide
- **Qt**: 使用 Qt 的命名约定（camelCase for functions, PascalCase for classes）
- **注释**: 英文注释，重要功能提供中文说明
- **提交信息**: 使用清晰的英文描述，格式：`type(scope): description`

## 📄 许可证

本项目使用 [MIT License](LICENSE) 开源许可证。

## 🙏 致谢

特别感谢以下开源项目和开发者：

- **[ZXing-C++](https://github.com/zxing-cpp/zxing-cpp)** - 优秀的跨平台条码处理库
- **[Qt](https://www.qt.io/)** - 强大的跨平台开发框架  
- **所有贡献者和用户** - 感谢每一份反馈和建议

### 技术支持
本项目的开发离不开以下技术和工具：
- **CMake** - 现代化的构建系统
- **Git** - 版本控制系统
- **GitHub** - 代码托管和协作平台
- **Qt Creator** - 集成开发环境

## 📞 联系方式

如有问题、建议或合作意向，请通过以下方式联系：

- **GitHub Issues**: [项目问题跟踪](https://github.com/toucheres/QRcode_Generate_Recongnition/issues)
- **邮件联系**: 
  - 主要联系：[761844639@qq.com]
  - 备用联系：[18161058362hjh@gmail.com]
- **项目主页**: [GitHub Repository](https://github.com/toucheres/QRcode_Generate_Recongnition)

### 反馈渠道
- **Bug报告**: 请使用GitHub Issues，并提供详细的重现步骤
- **功能建议**: 通过Issues提交功能请求，描述使用场景
- **技术支持**: 邮件联系，我们会尽快回复
- **商业合作**: 请发送邮件详细描述合作意向

---

## 📌 重要提示

- **开发状态**: 本项目处于活跃开发状态，功能持续更新中
- **稳定性**: v3.0版本已经过充分测试，适合生产环境使用
- **更新频率**: 定期发布新版本，建议关注项目获取最新功能
- **兼容性**: 向后兼容，旧版本生成的配置文件可在新版本中使用

**感谢您使用QR码生成识别器！** 🎉

*最后更新: 2024年*