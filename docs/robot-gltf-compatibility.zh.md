# 机器人 glTF 导入与原有模型的坐标兼容

普通 `ParaAsset.LoadParaX` 保持 `cp_old` 原有的 glTF/GLB 导入行为。机器人导出的模型需要左手转换时，必须显式使用新入口：

```lua
local ordinary = ParaAsset.LoadParaX("", "worlds/example/model.glb");
local robot = ParaAsset.LoadParaXWithGltfConversion("", "worlds/example/model.glb");
```

新入口仅对指定资产启用 Assimp 的 `aiProcess_MakeLeftHanded` 和 `aiProcess_FlipWindingOrder`。引擎世界坐标、普通模型、原有调用和 FBX 导入不因机器人模块启用而改变。

转换后的资产使用 `gltf-lh:` 前缀作为缓存标识，实际 `LocalFileName` 仍为原始文件路径。因此同一文件的普通导入和转换导入可以同时存在，加载顺序不会改变另一份模型。将转换资产的 `GetKeyName()` 交回 `LoadParaX` 可以保留该显式选项；卸载、重载继续使用同一选项。调用方不应把缓存标识当成物理文件路径使用。

该入口接受本地文件及已挂载世界中的 `.gltf`、`.glb`，不接受直接远程 URL。云端世界仍通过标准世界加载器下载或挂载，再使用世界内资源路径。不匹配的格式或直接远程 URL 返回无效资产。普通 `LoadParaX` 的远程加载流程保持不变。

机器人公共 Lua 必须同步使用这个入口。配套的 `RobotModelAsset.Load` 在新引擎上显式请求转换；在此前已默认转换 glTF 的机器人引擎上回退 `LoadParaX`。该回退不能使没有机器人原生接口的普通旧引擎获得机器人能力。发布新引擎时必须配套更新 Robot Lua，不能仅替换运行时。

本轮使用 Z 范围为 `[1, 2]` 的非对称三角形，分别生成 glTF 和 GLB。Windows x64 Debug 与 Web 基础 Release、机器人 Release 检查了普通优先、转换优先两种加载顺序，以及通过资产标识卸载重载：普通模型保持 `[1, 2]`，显式转换模型为 `[-2, -1]`。机器人 Web Debug/Release 的 MicroDuck 云端多实例加载与退出重入也通过。这里的 Windows 结果来自 `pararobot` 分支的同步修正；`cp_old` 自身的原生 Windows 配置仍缺 Boost log/locale，尚不能视为已构建通过。

修正与回归不得以修改全局坐标、全局导入开关、机器人编译宏或根据模型文件名猜测来替代。
