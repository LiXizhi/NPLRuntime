#!/bin/bash
# HarmonyOS 开发环境配置脚本
# 使用方法：source setup_env.sh

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}配置 HarmonyOS 开发环境...${NC}"

# 1. 设置 HarmonyOS SDK 路径
# 请根据你的实际安装路径修改
export HARMONYOS_SDK_HOME="/Volumes/APPLE_DATA/harmonyOS_sdk"

# 2. 设置 HarmonyOS NDK 路径（包含 API 版本）
# 注意：HarmonyOS SDK 的 native 目录在 API 版本目录下
export OHOS_NDK_HOME="${HARMONYOS_SDK_HOME}/18/native"

# 3. 获取当前脚本所在目录（即 boost 目录）
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 4. 设置 BOOST_ROOT（相对于脚本位置）
export BOOST_ROOT="${SCRIPT_DIR}/boost_src"

# 5. 验证路径
echo ""
echo -e "${YELLOW}验证环境配置...${NC}"
echo "HARMONYOS_SDK_HOME: $HARMONYOS_SDK_HOME"
echo "OHOS_NDK_HOME: $OHOS_NDK_HOME"
echo "BOOST_ROOT: $BOOST_ROOT"
echo ""

# 检查 SDK
if [ ! -d "$HARMONYOS_SDK_HOME" ]; then
    echo -e "${RED}[错误] HarmonyOS SDK 路径不存在: $HARMONYOS_SDK_HOME${NC}"
    echo "请修改脚本中的 HARMONYOS_SDK_HOME 路径"
    return 1
else
    echo -e "${GREEN}✓ HarmonyOS SDK 已找到${NC}"
fi

# 检查 NDK
if [ ! -d "$OHOS_NDK_HOME" ]; then
    echo -e "${RED}[错误] HarmonyOS NDK 路径不存在: $OHOS_NDK_HOME${NC}"
    echo "请确认 NDK 已安装在 SDK 的 native 目录下"
    return 1
else
    echo -e "${GREEN}✓ HarmonyOS NDK 已找到${NC}"
fi

# 检查 clang++
if [ ! -f "$OHOS_NDK_HOME/llvm/bin/clang++" ]; then
    echo -e "${YELLOW}[警告] clang++ 编译器未找到: $OHOS_NDK_HOME/llvm/bin/clang++${NC}"
    echo "请确认 NDK 安装完整"
else
    echo -e "${GREEN}✓ clang++ 编译器已找到${NC}"
fi

# 创建 BOOST_ROOT 目录（如果不存在）
if [ ! -d "$BOOST_ROOT" ]; then
    mkdir -p "$BOOST_ROOT"
    echo -e "${GREEN}✓ 已创建 BOOST_ROOT 目录${NC}"
fi

echo ""
echo -e "${GREEN}环境配置完成！${NC}"
echo ""
echo "下一步："
echo "  1. 运行构建脚本: ./build_harmonyos.sh"
echo "  2. 或使用 Python: python3 build_harmonyos.py"
echo ""
echo "提示：将以下内容添加到 ~/.zshrc 以永久保存配置："
echo ""
echo "export HARMONYOS_SDK_HOME=\"$HARMONYOS_SDK_HOME\""
echo "export OHOS_NDK_HOME=\"\${HARMONYOS_SDK_HOME}/18/native\""
echo "export BOOST_ROOT=\"$BOOST_ROOT\""
