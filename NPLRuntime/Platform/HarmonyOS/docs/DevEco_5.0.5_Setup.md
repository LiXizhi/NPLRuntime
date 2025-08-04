# DevEco Studio 5.0.5 快速设置指南

## 🎯 适用版本
本项目已针对 **DevEco Studio 5.0.5** 进行优化配置

## 🚀 快速开始

### 1. 环境要求
- ✅ DevEco Studio 5.0.5
- ✅ HarmonyOS SDK API 12
- ✅ Node.js 16+ 
- ✅ Java 17+

### 2. 打开项目
```bash
# 克隆或打开项目
cd /path/to/HarmonyOS

# DevEco Studio中: File → Open → 选择此目录
```

### 3. 首次构建
```bash
# 安装依赖
npm install

# 清理并构建
./hvigorw clean
./hvigorw assembleHap --mode debug
```

## 📋 关键配置说明

### SDK版本
- **compileSdkVersion**: "12"
- **compatibleSdkVersion**: "12" 
- **API级别**: HarmonyOS API 12

### 构建工具版本
- **hvigor**: 5.0.5
- **hvigor-ohos-plugin**: 5.0.5

### 产品变体
- `default`: 默认开发配置
- `tatfook`: Tatfook版本
- `palaka`: Palaka版本
- `papa`: Papa版本  
- `edu`: 教育版本

## 🔧 常见问题解决

### 问题1: "项目结构需要升级"
**解决方案**: 项目已升级到5.0.5兼容配置，重新同步即可

### 问题2: "SDK版本不正确"
**解决方案**: 
1. 确认DevEco Studio为5.0.5版本
2. 在SDK Manager中下载API 12
3. 检查local.properties中SDK路径

### 问题3: "构建失败"
**解决方案**:
```bash
# 清理重建
./hvigorw clean
rm -rf node_modules oh_modules
npm install
./hvigorw assembleHap --mode debug
```

### 问题4: "签名配置错误"
**解决方案**: 使用debug签名配置，DevEco Studio会自动生成调试证书

## 📱 运行测试

### 模拟器运行
1. 启动HarmonyOS模拟器
2. 点击DevEco Studio中的"Run"按钮
3. 选择目标设备

### 真机调试
1. 连接HarmonyOS设备
2. 开启开发者模式
3. 在DevEco Studio中选择设备运行

## 📚 相关文档

- [项目结构升级指南](./Project_Upgrade_Guide.md)
- [JNI到NAPI迁移总结](../JNI_to_NAPI_Migration_Summary.md)
- [DevEco设置指南](../DevEco_Setup_Guide.md)

## 🎉 验证成功标志

项目配置正确的标志：
- ✅ DevEco Studio正常打开项目
- ✅ 没有红色配置错误提示
- ✅ 能够选择构建变体
- ✅ 能够成功编译生成HAP包
- ✅ 能够在模拟器/设备上运行

现在您的项目已完全兼容DevEco Studio 5.0.5！
