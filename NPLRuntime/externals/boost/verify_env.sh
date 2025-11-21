#!/bin/bash
# HarmonyOS Boost 环境和构建验证脚本

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检查项计数
TOTAL_CHECKS=0
PASSED_CHECKS=0
FAILED_CHECKS=0
WARNING_CHECKS=0

# 打印标题
print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  HarmonyOS Boost 环境验证${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

# 检查函数
check_item() {
    local name=$1
    local condition=$2
    local error_msg=$3
    local is_warning=${4:-false}
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if [ "$condition" = "true" ]; then
        echo -e "${GREEN}✓${NC} $name"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        return 0
    else
        if [ "$is_warning" = "true" ]; then
            echo -e "${YELLOW}⚠${NC} $name"
            echo -e "  ${YELLOW}→ $error_msg${NC}"
            WARNING_CHECKS=$((WARNING_CHECKS + 1))
        else
            echo -e "${RED}✗${NC} $name"
            echo -e "  ${RED}→ $error_msg${NC}"
            FAILED_CHECKS=$((FAILED_CHECKS + 1))
        fi
        return 1
    fi
}

# 打印总结
print_summary() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}  验证总结${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo -e "总检查项: $TOTAL_CHECKS"
    echo -e "${GREEN}通过: $PASSED_CHECKS${NC}"
    echo -e "${YELLOW}警告: $WARNING_CHECKS${NC}"
    echo -e "${RED}失败: $FAILED_CHECKS${NC}"
    echo ""
    
    if [ $FAILED_CHECKS -eq 0 ]; then
        if [ $WARNING_CHECKS -eq 0 ]; then
            echo -e "${GREEN}✓ 所有检查通过！可以开始构建。${NC}"
        else
            echo -e "${YELLOW}⚠ 有一些警告，但可以尝试构建。${NC}"
        fi
        echo ""
        echo "下一步："
        echo "  1. 如果还未构建 Boost: ./build_harmonyos.sh"
        echo "  2. 构建 HarmonyOS 项目: cd ../../Platform/HarmonyOS && ./hvigorw assembleHap"
    else
        echo -e "${RED}✗ 有检查失败，请修复后再继续。${NC}"
    fi
    echo ""
}

# 主函数
main() {
    print_header
    
    # 1. 检查环境变量
    echo -e "${BLUE}[1] 检查环境变量${NC}"
    
    if [ -n "$HARMONYOS_SDK_HOME" ]; then
        check_item "HARMONYOS_SDK_HOME 已设置" "true"
    else
        check_item "HARMONYOS_SDK_HOME 已设置" "false" \
            "请设置: export HARMONYOS_SDK_HOME=/path/to/sdk"
    fi
    
    if [ -n "$OHOS_NDK_HOME" ]; then
        check_item "OHOS_NDK_HOME 已设置" "true"
    else
        check_item "OHOS_NDK_HOME 已设置" "false" \
            "请设置: export OHOS_NDK_HOME=\${HARMONYOS_SDK_HOME}/native"
    fi
    
    if [ -n "$BOOST_ROOT" ]; then
        check_item "BOOST_ROOT 已设置" "true"
    else
        check_item "BOOST_ROOT 已设置" "false" \
            "请设置: export BOOST_ROOT=/path/to/boost_src"
    fi
    
    echo ""
    echo -e "${BLUE}[2] 检查目录和文件${NC}"
    
    # 2. 检查 SDK 目录
    if [ -d "$HARMONYOS_SDK_HOME" ]; then
        check_item "HarmonyOS SDK 目录存在" "true"
    else
        check_item "HarmonyOS SDK 目录存在" "false" \
            "SDK 目录不存在: $HARMONYOS_SDK_HOME"
    fi
    
    # 3. 检查 NDK 目录
    if [ -d "$OHOS_NDK_HOME" ]; then
        check_item "HarmonyOS NDK 目录存在" "true"
    else
        check_item "HarmonyOS NDK 目录存在" "false" \
            "NDK 目录不存在: $OHOS_NDK_HOME"
    fi
    
    # 4. 检查编译器
    if [ -f "$OHOS_NDK_HOME/llvm/bin/clang++" ]; then
        check_item "clang++ 编译器存在" "true"
    else
        check_item "clang++ 编译器存在" "false" \
            "编译器不存在: $OHOS_NDK_HOME/llvm/bin/clang++"
    fi
    
    # 5. 检查 llvm-ar
    if [ -f "$OHOS_NDK_HOME/llvm/bin/llvm-ar" ]; then
        check_item "llvm-ar 归档工具存在" "true"
    else
        check_item "llvm-ar 归档工具存在" "false" \
            "归档工具不存在: $OHOS_NDK_HOME/llvm/bin/llvm-ar"
    fi
    
    # 6. 检查构建脚本
    SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    
    if [ -f "$SCRIPT_DIR/build_harmonyos.sh" ]; then
        check_item "build_harmonyos.sh 存在" "true"
    else
        check_item "build_harmonyos.sh 存在" "false" \
            "构建脚本不存在"
    fi
    
    if [ -f "$SCRIPT_DIR/build_harmonyos.py" ]; then
        check_item "build_harmonyos.py 存在" "true"
    else
        check_item "build_harmonyos.py 存在" "false" \
            "Python 构建脚本不存在"
    fi
    
    echo ""
    echo -e "${BLUE}[3] 检查 Python 环境${NC}"
    
    # 7. 检查 Python
    if command -v python3 &> /dev/null; then
        PYTHON_VERSION=$(python3 --version 2>&1 | awk '{print $2}')
        check_item "Python 3 已安装 ($PYTHON_VERSION)" "true"
    else
        check_item "Python 3 已安装" "false" \
            "未找到 python3 命令"
    fi
    
    echo ""
    echo -e "${BLUE}[4] 检查 Boost 构建状态${NC}"
    
    # 8. 检查 BOOST_ROOT 目录
    if [ -d "$BOOST_ROOT" ]; then
        check_item "BOOST_ROOT 目录存在" "true"
        
        # 9. 检查是否有源码
        if [ -f "$BOOST_ROOT/bootstrap.sh" ]; then
            check_item "Boost 源码已下载" "true"
        else
            check_item "Boost 源码已下载" "false" \
                "源码未下载，将在构建时自动下载" "true"
        fi
    else
        check_item "BOOST_ROOT 目录存在" "false" \
            "目录不存在，将在构建时自动创建" "true"
    fi
    
    # 10. 检查预构建库
    PREBUILD_DIR="$SCRIPT_DIR/prebuild/harmonyos"
    
    if [ -d "$PREBUILD_DIR/include/boost" ]; then
        check_item "Boost 头文件已构建" "true"
    else
        check_item "Boost 头文件已构建" "false" \
            "头文件未构建，需要运行 ./build_harmonyos.sh" "true"
    fi
    
    if [ -d "$PREBUILD_DIR/arm64-v8a" ] && [ "$(ls -A $PREBUILD_DIR/arm64-v8a 2>/dev/null)" ]; then
        LIB_COUNT=$(ls -1 "$PREBUILD_DIR/arm64-v8a"/*.a 2>/dev/null | wc -l)
        check_item "arm64-v8a 库已构建 ($LIB_COUNT 个库)" "true"
    else
        check_item "arm64-v8a 库已构建" "false" \
            "库未构建，需要运行 ./build_harmonyos.sh" "true"
    fi
    
    if [ -d "$PREBUILD_DIR/armeabi-v7a" ] && [ "$(ls -A $PREBUILD_DIR/armeabi-v7a 2>/dev/null)" ]; then
        LIB_COUNT=$(ls -1 "$PREBUILD_DIR/armeabi-v7a"/*.a 2>/dev/null | wc -l)
        check_item "armeabi-v7a 库已构建 ($LIB_COUNT 个库)" "true"
    else
        check_item "armeabi-v7a 库已构建" "false" \
            "库未构建，需要运行 ./build_harmonyos.sh" "true"
    fi
    
    echo ""
    echo -e "${BLUE}[5] 检查 CMake 集成${NC}"
    
    # 11. 检查 CMakeLists.txt
    CMAKE_FILE="$SCRIPT_DIR/../../Platform/HarmonyOS/entry/src/main/cpp/CMakeLists.txt"
    if [ -f "$CMAKE_FILE" ]; then
        check_item "CMakeLists.txt 存在" "true"
        
        # 检查是否包含 Boost 配置
        if grep -q "BOOST_ROOT" "$CMAKE_FILE"; then
            check_item "CMakeLists.txt 已集成 Boost" "true"
        else
            check_item "CMakeLists.txt 已集成 Boost" "false" \
                "CMakeLists.txt 未配置 Boost" "true"
        fi
    else
        check_item "CMakeLists.txt 存在" "false" \
            "CMakeLists.txt 不存在"
    fi
    
    echo ""
    echo -e "${BLUE}[6] 检查磁盘空间${NC}"
    
    # 12. 检查磁盘空间
    AVAILABLE_SPACE=$(df -h "$SCRIPT_DIR" | awk 'NR==2 {print $4}')
    AVAILABLE_SPACE_GB=$(df -k "$SCRIPT_DIR" | awk 'NR==2 {print int($4/1024/1024)}')
    
    if [ "$AVAILABLE_SPACE_GB" -gt 10 ]; then
        check_item "磁盘空间充足 ($AVAILABLE_SPACE 可用)" "true"
    else
        check_item "磁盘空间充足 ($AVAILABLE_SPACE 可用)" "false" \
            "建议至少有 10GB 可用空间，当前只有 ${AVAILABLE_SPACE}" "true"
    fi
    
    # 打印总结
    print_summary
}

# 运行主函数
main
