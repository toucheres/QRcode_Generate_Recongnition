# 安装指南

本文档详细介绍了QR码生成识别器的安装和配置过程。

## 📋 系统要求

### 最低要求
- **操作系统**: Windows 10, Ubuntu 18.04, macOS 10.14
- **内存**: 4GB RAM
- **存储空间**: 500MB 可用空间
- **显卡**: 支持OpenGL 2.0的显卡

### 推荐配置
- **操作系统**: Windows 11, Ubuntu 20.04+, macOS 11+
- **内存**: 8GB RAM
- **存储空间**: 1GB 可用空间
- **摄像头**: 用于实时识别功能

## 🛠️ 开发环境安装

### Windows

#### 1. 安装Visual Studio 2019/2022
```cmd
# 下载并安装Visual Studio Community
# 确保选择以下组件：
# - MSVC v143 - VS 2022 C++ x64/x86 build tools
# - Windows 10/11 SDK
# - CMake tools for VS
```

#### 2. 安装Qt6
```cmd
# 方法1：使用官方安装器
# 下载 qt-online-installer-windows.exe
# 选择Qt 6.5+ LTS版本，包含以下组件：
# - Qt Core, Widgets, Multimedia, MultimediaWidgets
# - Qt SVG, Core5Compat

# 方法2：使用vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install qt6[core,widgets,multimedia] --triplet x64-windows
```

#### 3. 设置环境变量
```cmd
# 添加到系统环境变量
set QTDIR=C:\Qt\6.5.0\msvc2019_64
set PATH=%QTDIR%\bin;%PATH%
set Qt6_DIR=%QTDIR%\lib\cmake\Qt6
```

### Ubuntu/Debian

#### 1. 安装开发工具
```bash
sudo apt update
sudo apt install build-essential cmake git

# 安装Qt6
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-svg-dev
sudo apt install libqt6multimedia6 libqt6multimediawidgets6
```

#### 2. 安装额外依赖
```bash
# 摄像头支持
sudo apt install libv4l-dev

# 图像处理库
sudo apt install libpng-dev libjpeg-dev
```

### macOS

#### 1. 安装Xcode
```bash
# 从App Store安装Xcode
# 或安装Command Line Tools
xcode-select --install
```

#### 2. 安装Homebrew和Qt6
```bash
# 安装Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装Qt6
brew install qt6
brew install cmake
```

#### 3. 设置环境变量
```bash
# 添加到 ~/.zshrc 或 ~/.bash_profile
export QTDIR=$(brew --prefix qt6)
export PATH=$QTDIR/bin:$PATH
export Qt6_DIR=$QTDIR/lib/cmake/Qt6
```

## 🔧 构建项目

### 1. 克隆源码
```bash
git clone https://github.com/toucheres/QRcode_Generate_Recongnition.git
cd QRcode_Generate_Recongnition
```

### 2. 初始化子模块
```bash
git submodule update --init --recursive
```

### 3. 创建构建目录
```bash
mkdir build
cd build
```

### 4. 配置和构建

#### Windows (Visual Studio)
```cmd
# 配置
cmake .. -G "Visual Studio 17 2022" -A x64
# 或指定Qt路径
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=%QTDIR%

# 构建
cmake --build . --config Release

# 运行
.\Release\QRcode_Generator_Recongniser.exe
```

#### Windows (MinGW)
```cmd
# 配置
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 构建
cmake --build .

# 运行
.\QRcode_Generator_Recongniser.exe
```

#### Linux/macOS
```bash
# 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建
make -j$(nproc)

# 运行
./QRcode_Generator_Recongniser
```

### 5. 安装（可选）
```bash
# Linux/macOS
sudo make install

# Windows - 复制到Program Files
cmake --install . --prefix "C:\Program Files\QRCodeApp"
```

## 📦 依赖库说明

### ZXing-C++
项目已包含ZXing-C++作为子模块，无需单独安装。如果遇到问题：

```bash
# 手动更新ZXing子模块
git submodule update --remote 3rd/zxing

# 或重新克隆
rm -rf 3rd/zxing
git submodule add https://github.com/zxing-cpp/zxing-cpp.git 3rd/zxing
```

### Qt6组件
确保安装了以下Qt6组件：
- **Qt6Core**: 核心功能
- **Qt6Widgets**: UI组件
- **Qt6Multimedia**: 多媒体支持
- **Qt6MultimediaWidgets**: 多媒体UI组件
- **Qt6Svg**: SVG支持
- **Qt6Core5Compat**: Qt5兼容层

## 🚨 常见安装问题

### 1. Qt6找不到
```bash
# 检查Qt6安装
qmake --version

# 设置Qt6路径
export Qt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

### 2. CMake版本过低
```bash
# Ubuntu - 安装新版CMake
sudo snap install cmake --classic

# 或从源码编译
wget https://cmake.org/files/v3.28/cmake-3.28.0.tar.gz
tar -xzf cmake-3.28.0.tar.gz
cd cmake-3.28.0
./bootstrap && make && sudo make install
```

### 3. 编译器兼容性
```bash
# 检查C++17支持
g++ --version  # 需要GCC 7+
clang++ --version  # 需要Clang 5+
```

### 4. 摄像头权限问题

#### Windows
- 检查"设置" → "隐私" → "摄像头"
- 确保允许桌面应用访问摄像头

#### macOS
- 系统会自动弹出权限请求
- 或在"系统偏好设置" → "安全性与隐私" → "摄像头"中手动添加

#### Linux
```bash
# 检查摄像头设备
ls /dev/video*

# 添加用户到video组
sudo usermod -a -G video $USER

# 重新登录生效
```

## 📱 部署说明

### Windows部署
```cmd
# 收集依赖库
windeployqt.exe --debug --compiler-runtime QRcode_Generator_Recongniser.exe

# 创建安装包（使用NSIS或Inno Setup）
```

### Linux AppImage
```bash
# 使用linuxdeployqt
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxdeployqt-continuous-x86_64.AppImage

./linuxdeployqt-continuous-x86_64.AppImage QRcode_Generator_Recongniser -appimage
```

### macOS应用包
```bash
# 使用macdeployqt
macdeployqt QRcode_Generator_Recongniser.app

# 创建DMG
hdiutil create -volname "QR Code Generator" -srcfolder QRcode_Generator_Recongniser.app -ov QRCodeApp.dmg
```

## ✅ 安装验证

安装完成后，运行以下检查：

1. **启动应用**：确保程序能正常启动
2. **生成功能**：测试生成QR码
3. **识别功能**：测试图片识别
4. **摄像头功能**：测试实时识别（如果有摄像头）
5. **设置保存**：测试设置的保存和加载

如果所有功能正常，说明安装成功！

---

如有安装问题，请查看[故障排除文档](TROUBLESHOOTING.md)或通过GitHub Issues寻求帮助。
