# QR码生成识别器 v2.0

一个基于Qt6和ZXing-C++库开发的现代化QR码生成和识别应用程序，采用模块化架构设计。

## 🎯 功能特性

### 二维码生成
- ✅ 支持多种二维码格式（QR Code, Micro QR等）
- ✅ 四级错误纠正级别支持（L, M, Q, H）
- ✅ 自定义尺寸设置（100px - 1000px）
- ✅ Logo嵌入功能，可调节Logo大小
- ✅ 支持中文、日文、韩文等UTF-8字符
- ✅ 多种格式导出（PNG, JPEG, BMP, SVG）

### 二维码识别（开发中）
- 🔄 图片文件识别
- 🔄 摄像头实时识别
- 🔄 批量识别处理
- 🔄 识别结果轮廓显示

### 用户界面
- ✅ 现代化Material Design风格界面
- ✅ 标签页式多功能布局
- ✅ 响应式设计，适配不同屏幕尺寸
- ✅ 支持键盘快捷键
- ✅ 状态栏实时反馈

## 🏗️ 技术架构

### 核心技术栈
- **Qt6** - 跨平台GUI框架
- **ZXing-C++** - 高性能条码处理库
- **CMake 3.16+** - 现代化构建系统
- **C++17** - 现代C++标准

### 模块化架构

```
src/
├── core/                   # 核心业务逻辑
│   ├── QRCodeGenerator.*   # 二维码生成器
│   └── QRCodeRecognizer.*  # 二维码识别器
├── gui/                    # 用户界面组件
│   ├── BaseWidget.*        # UI基类
│   ├── GeneratorWidget.*   # 生成器界面
│   └── MainWindow.*        # 主窗口
├── utils/                  # 工具类
│   └── AppUtils.*          # 应用工具函数
└── main.cpp               # 程序入口

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

## 安装和构建

### 1. 克隆项目

```bash
git clone <https://github.com/toucheres/QRcode_Generate_Recongnition>
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
- Windows: `build/Release/QRcode_Generate_Recongnise.exe`
- Linux/macOS: `build/QRcode_Generate_Recongnise`

## 使用说明

1. **启动应用程序**
   - 运行编译后的可执行文件

2. **生成QR码**
   - 在文本输入框中输入要编码的文本
   - 支持中文、英文、数字、符号等UTF-8字符
   - 点击"生成二维码"按钮或按回车键
   - 生成的QR码将显示在下方区域

3. **功能特点**
   - 实时输入验证
   - UTF-8字符完全支持
   - 错误提示和成功确认
   - 调试信息输出（开发模式）

## 开发说明

### 代码架构

- **MainWindow**: 主窗口类，负责UI管理和用户交互
- **generateQRCodePixmap()**: 核心QR码生成函数
- **zxingMatrixToQPixmap()**: ZXing BitMatrix到Qt QPixmap的转换

### 关键技术细节

1. **UTF-8支持**
   ```cpp
   // 确保UTF-8编码正确处理
   QByteArray utf8Data = text.toUtf8();
   std::string stdText = utf8Data.toStdString();
   writer.setEncoding(ZXing::CharacterSet::UTF8);
   ```

2. **错误处理**
   ```cpp
   try {
       // QR码生成逻辑
   } catch (const std::exception& e) {
       // 用户友好的错误提示
   }
   ```

3. **跨平台兼容性**
   - 使用CMake处理不同平台的构建差异
   - Qt6提供跨平台GUI支持

### 调试模式

开发时启用详细调试输出：
```cpp
qDebug() << "Input text:" << text;
qDebug() << "UTF-8 bytes:" << text.toUtf8().toHex();
```

## 版本历史

### v1.0.0 (当前版本)
- ✅ 基本QR码生成功能
- ✅ UTF-8字符支持
- ✅ 图形用户界面
- ✅ 错误处理和用户反馈

### 未来计划
- 🔄 QR码识别功能
- 🔄 批量生成
- 🔄 自定义样式选项
- 🔄 文件导出功能

## 常见问题

### Q: 编译时找不到Qt6
**A**: 确保设置了正确的QTDIR环境变量，并且Qt6已正确安装。

### Q: 中文字符显示为乱码
**A**: 确保源代码文件以UTF-8编码保存，并且编译器支持UTF-8。

### Q: 生成的QR码无法扫描
**A**: 检查输入文本是否过长，建议控制在适当长度内。QR码有容量限制。

## 贡献指南

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 许可证

本项目使用 [MIT License](LICENSE)。

## 致谢

- [ZXing-C++](https://github.com/zxing-cpp/zxing-cpp) - 优秀的条码处理库
- [Qt](https://www.qt.io/) - 强大的跨平台开发框架

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 Issue
- 发送邮件至：[761844639@qq.com]
- 或：[18161058362hjh@gmail.com]
---

**注意**: 本项目仍在开发中，功能可能会有所变化。建议定期更新到最新版本。
```