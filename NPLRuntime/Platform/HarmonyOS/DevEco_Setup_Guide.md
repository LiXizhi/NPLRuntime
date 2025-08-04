# DevEco Studio 项目配置修复指南

## 问题描述
在DevEco Studio中打开项目时出现错误：
```
Unable to find the default product. Add one to the build-profile.json5 file.
```

## 已修复的问题

### 1. build-profile.json5 语法错误
- ✅ 修复了debug配置中多余的逗号
- ✅ 添加了"default"产品配置作为默认产品
- ✅ 更新了模块targets配置

### 2. 签名配置
- ✅ 添加了debug签名配置用于开发调试
- ✅ 创建了cert目录用于存放证书文件

### 3. 模块配置
- ✅ 创建了entry/hvigorfile.ts文件
- ✅ 确保所有模块都正确配置

## 如何在DevEco Studio中使用

### 1. 打开项目
1. 启动DevEco Studio
2. 选择 "Open" 
3. 选择项目根目录：`/Volumes/CODE/NPLRuntime/NPLRuntime/Platform/HarmonyOS`

### 2. 配置SDK
确保已安装HarmonyOS SDK API 12：
1. File -> Settings -> HarmonyOS SDK
2. 确认SDK路径正确
3. 下载并安装API 12 SDK

### 3. 生成调试证书
首次运行前需要生成调试证书：
1. Build -> Generate Key and CSR
2. 或使用DevEco Studio自动生成的调试证书

### 4. 同步项目
1. 点击 "Sync Now" 或 Sync Project
2. 等待依赖下载完成

### 5. 选择构建变体
在构建配置中选择：
- Product: `default` (推荐用于开发)
- Build Mode: `debug`

## 构建配置说明

### 产品变体
- `default`: 默认配置，用于开发调试
- `tatfook`: Tatfook版本
- `palaka`: Palaka版本  
- `papa`: Papa版本
- `edu`: 教育版本

### 构建模式
- `debug`: 调试模式，包含调试信息
- `release`: 发布模式，优化构建

## 构建命令

### 使用DevEco Studio GUI
1. Build -> Make Project
2. Build -> Build Hap(s)

### 使用命令行
```bash
# 进入项目根目录
cd /Volumes/CODE/NPLRuntime/NPLRuntime/Platform/HarmonyOS

# 构建调试版本
npm run build:debug

# 构建发布版本  
npm run build:release

# 清理构建
npm run clean
```

## 故障排除

### 如果仍然出现"Unable to find the default product"错误：
1. 检查build-profile.json5语法是否正确
2. 确认products数组中包含"default"产品
3. 重启DevEco Studio
4. 清理项目缓存：Build -> Clean Project

### 如果出现签名错误：
1. 在DevEco Studio中自动生成调试证书
2. 或手动配置签名文件路径

### 如果出现SDK版本错误：
1. 检查compileSdkVersion和compatibleSdkVersion
2. 确保安装了对应版本的HarmonyOS SDK

## 当前项目状态
- ✅ 基础项目结构完整
- ✅ ArkTS代码已迁移（EntryAbility等）
- ✅ C++ NAPI代码已迁移
- ✅ 构建配置已修复
- ⏳ 等待ParaEngine核心集成
- ⏳ 等待完整功能测试

现在可以在DevEco Studio中正常打开和构建项目了！
