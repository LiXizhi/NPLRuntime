# 15 NPLRuntime：引擎底座与版本依赖

本文由 AI 根据第二大脑资料与 Git 源码整理。核对日期：2026-09-17。代码基线：`NPLRuntime` / `dev` / `e732789d0199`。这是源码快照说明；未据此宣称生产环境已完成全部验收。

## 在产品中的位置

NPLRuntime 源仓包含 ParaEngine C++ 引擎与 NPL 运行时；Lua/NPL 应用脚本主要在 ParaWorld 仓，Web 分发在 webparacraft 仓。三者要匹配版本，不能只更新脚本就推断所有原生 API 可用。

## 架构

```text
应用脚本 → ParaScriptBindings → NPL Runtime 状态与消息分发
                           → 场景 / 资源 / 方块 / 物理 / GUI
                           → 渲染设备 / 平台窗口与音频
```

`Client/CMakeLists.txt` 组织客户端编译依赖。客户端与 headless server 有不同构建路径；平台图形能力、插件、资源包和脚本 bootstrap 决定最终产品行为。源码文档分别覆盖运行时、脚本绑定、场景和渲染。

## AI 阅读顺序

先读 `docs/README.md` 与 `docs/architecture.md`，再按问题进入 `docs/npl-runtime.md`、`docs/script-bindings.md`、`docs/scene-graph-and-3d-engine.md`；最后追对应 C++ 与 Lua 调用方。不要从项目名中的 Neural 推导为某种已训练大模型。

## 特别版本提示

本机该仓 HEAD 为 2026-07-03，而 ParaWorld 机器人脚本更新到 2026-09；本轮在此 NPLRuntime 的 ParaScriptBindings 中未找到 ParaMuJoCo 匹配，因此不能把该 checkout 当作最新 MuJoCo 二进制的构建来源。另有 NPLRuntimeDev checkout，未在本轮建立其与实际发布二进制的对应关系。

交付时记录引擎 commit/二进制版本、脚本 commit、资源包版本、平台和世界包，再运行对应 API smoke test。本文是现有源码导航，不是当前生产构建可复现性证明。

## 源码与维护入口

- [`Client/CMakeLists.txt`](https://github.com/LiXizhi/NPLRuntime/blob/e732789d0199eb3c573632f22627a46d936e6933/Client/CMakeLists.txt)
- [`docs/architecture.md`](https://github.com/LiXizhi/NPLRuntime/blob/e732789d0199eb3c573632f22627a46d936e6933/docs/architecture.md)
- [`docs/npl-runtime.md`](https://github.com/LiXizhi/NPLRuntime/blob/e732789d0199eb3c573632f22627a46d936e6933/docs/npl-runtime.md)
- [`docs/script-bindings.md`](https://github.com/LiXizhi/NPLRuntime/blob/e732789d0199eb3c573632f22627a46d936e6933/docs/script-bindings.md)
- [`docs/scene-graph-and-3d-engine.md`](https://github.com/LiXizhi/NPLRuntime/blob/e732789d0199eb3c573632f22627a46d936e6933/docs/scene-graph-and-3d-engine.md)

## 钉钉发布版本

[国基知识库中的本篇手册](https://alidocs.dingtalk.com/i/nodes/9bN7RYPWdeE4RAedFj3jOlkLVZd1wyK0)（2026-09-17）。截图保存在钉钉原文中；后续更新本文件后同步原文档。
