# DevEco Studio 5.0.5 项目配置升级指南

## 问题描述
```
Cause: The project structure and configuration require an upgrade. 
Solution: 1. Use Migrate Assistant to auto-upgrade the project structure and configuration. 
2. Manually upgrade the project structure and configuration by following the instructions.
```

## 升级到DevEco Studio 5.0.5兼容配置

根据华为官方迁移指南，已将项目升级为DevEco Studio 5.0.5兼容配置。

## 已完成的升级

### 1. 核心配置文件升级

#### build-profile.json5 (根目录)
- ✅ 添加了缺失的 `compileSdkVersion` 字段
- ✅ 使用DevEco Studio 5.0.5兼容的SDK版本格式: "12"
- ✅ 包含完整的产品配置
- ✅ 正确的签名配置

#### entry/build-profile.json5
- ✅ 添加了 `apiType: "stageMode"`
- ✅ 添加了 `buildOption` 配置
- ✅ 配置了C++编译选项
- ✅ 设置了ABI过滤器

#### paracraft_core/build-profile.json5  
- ✅ 添加了 `apiType: "stageMode"`
- ✅ 统一的模块配置

### 2. 构建工具升级

#### package.json
- ✅ 使用DevEco Studio 5.0.5兼容版本: hvigor 5.0.5
- ✅ 升级hvigor-ohos-plugin版本: 5.0.5

#### hvigorfile.ts文件
- ✅ 根目录hvigorfile.ts
- ✅ entry/hvigorfile.ts  
- ✅ paracraft_core/hvigorfile.ts

### 3. 构建脚本
- ✅ 创建了hvigorw (Unix/Linux/macOS)
- ✅ 创建了hvigorw.bat (Windows)
- ✅ 配置了DevEco Studio 5.0.5兼容的构建环境

### 4. 项目结构文件
- ✅ 创建了.gitignore文件
- ✅ 配置了开发环境忽略规则
- ✅ 保留了local.properties配置
- ✅ 创建了.gitignore文件
- ✅ 配置了开发环境忽略规则

## 升级后的项目结构

**⚠️ 最新状态 (2025.8.4):**
使用DevEco Studio内置的hvigor 5.17.4时，项目需要进一步升级：

1. ✅ 修复了SDK版本配置：使用整数类型而非字符串
2. ✅ 添加了根目录的 `oh-package.json5` 文件  
3. ✅ 更新了hvigor-config.json5版本为5.17.4
4. ⚠️ 仍需解决：项目结构升级问题

**当前错误：**
```
The project structure and configuration need to be upgraded before use.
Use the migration tool to update the project structure and configuration with Deveco Studio
```

**解决方案：**
需要在DevEco Studio中使用Migration Assistant进行自动升级，或参考最新的项目结构指南。

```
HarmonyOS/
├── .gitignore                    # Git忽略文件
├── build-profile.json5           # 根构建配置
├── hvigorfile.ts                # 根构建脚本
├── package.json                 # 依赖配置
├── AppScope/                    # 应用范围配置
├── entry/                       # 主模块
│   ├── build-profile.json5      # 模块构建配置
│   ├── hvigorfile.ts           # 模块构建脚本
│   └── src/main/
│       ├── ets/                # ArkTS代码
│       └── cpp/                # Native C++代码
├── paracraft_core/             # 核心库模块
│   ├── build-profile.json5     # 库构建配置
│   ├── hvigorfile.ts          # 库构建脚本
│   └── src/
└── docs/                       # 文档
```

## 关键配置说明

### API类型
所有模块都使用Stage模型：
```json
{
  "apiType": "stageMode"
}
```

### SDK版本
统一使用DevEco Studio 5.0.5兼容的API 12格式：
```json
{
  "compileSdkVersion": "12",
  "compatibleSdkVersion": "12"
}
```

### C++编译配置
支持ARM64和ARM32架构：
```json
{
  "buildOption": {
    "externalNativeOptions": {
      "path": "./src/main/cpp/CMakeLists.txt",
      "abiFilters": ["arm64-v8a", "armeabi-v7a"]
    }
  }
}
```

## 在DevEco Studio 5.0.5中使用

### 1. 检查DevEco Studio版本
确保使用DevEco Studio 5.0.5或兼容版本：
- Help → About DevEco Studio
- 检查版本号是否为5.0.5.x

### 2. 同步依赖
```bash
# 安装DevEco Studio 5.0.5兼容的构建工具
npm install
```

### 3. 重新导入项目
1. 关闭DevEco Studio
2. 删除.idea文件夹 (如果存在)
3. 重新打开项目

### 4. 配置SDK
确保安装了HarmonyOS API 12 SDK：
1. File → Settings → HarmonyOS SDK
2. 下载API 12 SDK
3. 确认SDK路径在local.properties中正确配置

### 5. 构建项目
```bash
# 清理构建
./hvigorw clean

# 构建调试版本
./hvigorw assembleHap --mode debug
```

## 验证升级结果

### 检查项目配置
- [ ] DevEco Studio正常打开项目
- [ ] 没有红色配置错误
- [ ] 能够选择构建变体
- [ ] 能够成功同步项目

### 检查构建功能
- [ ] 能够编译ArkTS代码
- [ ] 能够编译C++代码  
- [ ] 能够生成HAP包
- [ ] 能够在模拟器/设备上运行

## 故障排除

### 如果仍有升级错误：
1. 使用DevEco Studio的Migrate Assistant
2. 检查SDK版本是否正确安装
3. 清理项目缓存重新构建
4. 检查网络连接，确保能下载依赖

### 如果构建失败：
1. 检查CMakeLists.txt配置
2. 确认NDK路径正确
3. 检查ArkTS语法兼容性
4. 查看详细错误日志

现在项目应该符合最新的HarmonyOS项目结构要求！
