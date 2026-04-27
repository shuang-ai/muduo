#!/bin/bash
# 设置脚本在遇到任何错误时立即退出（非零返回值）
set -e

# ==================== 配置变量 ====================
# 获取脚本所在目录的绝对路径（解决软链接问题）
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# 构建目录：项目根目录下的 build 文件夹
BUILD_DIR="${PROJECT_ROOT}/build"
# 头文件安装目标路径（系统目录，需要 sudo 权限）
INCLUDE_INSTALL_DIR="/usr/include/mymuduo"
# 动态库安装目标路径（系统库目录）
LIB_INSTALL_DIR="/usr/lib"
# 生成的库文件名
LIB_NAME="libmy_muduo.so"

# ==================== 编译构建项目 ====================
# 创建 build 目录（-p 选项：如果目录已存在不报错）
mkdir -p "$BUILD_DIR"
# 清空 build 目录下的所有内容（${BUILD_DIR:?} 防止变量为空导致误删根目录）
rm -rf "${BUILD_DIR:?}"/*
# 进入 build 目录
cd "$BUILD_DIR"
# 运行 CMake 生成 Makefile（.. 表示上一级目录，即项目根目录）
cmake ..
# 并行编译：-j$(nproc) 使用所有 CPU 核心加速编译
make -j"$(nproc)"
# 返回项目根目录
cd "$PROJECT_ROOT"

# ==================== 安装头文件 ====================
echo "Installing headers to $INCLUDE_INSTALL_DIR..."
# 创建目标头文件目录（需要 sudo 权限）
sudo mkdir -p "$INCLUDE_INSTALL_DIR"
# 头文件源目录
HEADER_DIR="${PROJECT_ROOT}/include/mymuduo"
# 如果头文件目录存在
if [ -d "$HEADER_DIR" ]; then
    # 遍历目录下所有 .h 头文件
    for header in "$HEADER_DIR"/*.h; do
        # 检查文件是否存在（防止通配符无匹配时出错）
        [ -e "$header" ] && sudo cp "$header" "$INCLUDE_INSTALL_DIR/"
    done
fi

# ==================== 安装动态库 ====================
# 检查库文件是否生成成功
if [ -f "${PROJECT_ROOT}/lib/${LIB_NAME}" ]; then
    echo "Installing library to $LIB_INSTALL_DIR..."
    # 复制库文件到系统目录
    sudo cp "${PROJECT_ROOT}/lib/${LIB_NAME}" "$LIB_INSTALL_DIR/"
    # 更新动态链接器缓存（让系统能找到新安装的库）
    sudo ldconfig
else
    # 库文件不存在时给出警告
    echo "Warning: ${PROJECT_ROOT}/lib/${LIB_NAME} not found."
fi

# 完成提示
echo "Build and installation completed."