# 开发者文档

## 架构设计

### 设计原则

1. **单一职责原则**: 每个类只负责一个特定的功能
2. **开放封闭原则**: 对扩展开放，对修改封闭
3. **依赖倒置原则**: 依赖抽象而不是具体实现
4. **模块化设计**: 清晰的模块边界和接口

### 模块划分

#### Core模块 (核心业务逻辑)
- **QRCodeGenerator**: 二维码生成核心逻辑
  - 支持多种格式和错误纠正级别
  - Logo嵌入功能
  - 错误处理和验证

- **QRCodeRecognizer**: 二维码识别核心逻辑
  - 同步和异步识别
  - 多线程处理
  - 结果位置信息

#### GUI模块 (用户界面)
- **BaseWidget**: UI组件基类
  - 统一的样式管理
  - 通用布局创建方法
  - 组件工厂方法

- **GeneratorWidget**: 生成器界面
  - 参数配置界面
  - 实时预览
  - 文件保存

- **MainWindow**: 主窗口
  - 整体布局管理
  - 菜单和状态栏
  - 组件间通信协调

#### Utils模块 (工具类)
- **AppUtils**: 应用工具函数
  - 文件格式验证
  - URL验证
  - 字符串处理

### 数据流向

```
User Input -> GeneratorWidget -> QRCodeGenerator -> ZXing-C++ -> Result
              ↓
           UI Update <- MainWindow <- Signal/Slot <- Generated QR Code
```

## 编码规范

### 命名约定

- **类名**: PascalCase (如: `QRCodeGenerator`)
- **方法名**: camelCase (如: `generateQRCode`)
- **变量名**: camelCase (如: `qrCodePixmap`)
- **成员变量**: m_前缀 + camelCase (如: `m_generator`)
- **常量**: UPPER_SNAKE_CASE (如: `MAX_QUEUE_SIZE`)

### 文件组织

- 头文件(.h): `include/模块/类名.h`
- 源文件(.cpp): `src/模块/类名.cpp`
- 每个类一个文件
- 文件名与类名一致

### 注释规范

使用Doxygen风格注释：

```cpp
/**
 * @brief 方法简要描述
 * @param param1 参数1描述
 * @param param2 参数2描述
 * @return 返回值描述
 */
ReturnType methodName(Type1 param1, Type2 param2);
```

## 扩展指南

### 添加新的二维码格式

1. 在`QRCodeGenerator::GenerationConfig`中添加新格式
2. 更新`convertErrorCorrectionLevel`方法
3. 在`GeneratorWidget`中添加UI选项
4. 测试新格式的生成和识别

### 添加新的UI组件

1. 继承`BaseWidget`类
2. 实现`setupUI`方法
3. 重写`applyDefaultStyles`方法
4. 在`MainWindow`中集成新组件

### 添加新的工具函数

1. 在`AppUtils`类中添加静态方法
2. 添加相应的单元测试
3. 更新文档

## 性能优化

### 二维码生成优化

- 缓存常用配置的结果
- 异步生成大尺寸二维码
- 使用合适的图像格式

### 二维码识别优化

- 图像预处理优化
- 多线程识别
- 队列管理避免内存泄漏

### UI响应性优化

- 避免在主线程进行耗时操作
- 使用异步信号槽
- 适当的进度反馈

## 测试策略

### 单元测试

- 核心算法测试
- 边界条件测试
- 错误处理测试

### 集成测试

- 组件间通信测试
- 文件保存/加载测试
- UI交互测试

### 性能测试

- 大量二维码生成测试
- 内存使用测试
- 响应时间测试

## 调试技巧

### 日志输出

使用QDebug进行调试输出：

```cpp
qDebug() << "生成二维码:" << config.text;
```

### 内存检查

- 使用智能指针管理内存
- 检查父子关系正确性
- 避免循环引用

### 性能分析

- 使用Qt Creator的性能分析工具
- 监控CPU和内存使用
- 识别性能瓶颈

## 部署说明

### Windows部署

1. 使用windeployqt工具
2. 打包必要的Qt库
3. 创建安装程序

### Linux部署

1. 使用AppImage格式
2. 或创建deb/rpm包
3. 处理依赖关系

### macOS部署

1. 创建.app包
2. 代码签名
3. 公证处理

## 常见问题

### 编译问题

**Q: ZXing链接错误**
A: 确保ZXing_WRITERS和ZXING_READERS都设置为ON

**Q: Qt6找不到**
A: 设置Qt6_DIR环境变量指向Qt安装目录

### 运行时问题

**Q: 中文显示乱码**
A: 确保设置了UTF-8编码，检查字体支持

**Q: 二维码识别失败**
A: 检查图像质量，尝试不同的识别参数

## 贡献流程

1. Fork项目
2. 创建功能分支
3. 编写代码和测试
4. 提交Pull Request
5. 代码审查
6. 合并到主分支

## 版本发布

### 版本号规则

采用语义化版本号: MAJOR.MINOR.PATCH

- MAJOR: 不兼容的API修改
- MINOR: 向后兼容的功能新增
- PATCH: 向后兼容的问题修正

### 发布检查清单

- [ ] 所有测试通过
- [ ] 文档更新
- [ ] 版本号更新
- [ ] 生成发布包
- [ ] 创建Git标签
- [ ] 发布说明
