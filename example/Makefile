# 编译器
CXX = g++
# 编译选项
CXXFLAGS = -std=c++17 -g -Wall -O0
# 链接选项
LDFLAGS = -lpthread

# 目标可执行文件
TARGET = echo_server
# 源文件
SRCS = echo_server.cc

# 头文件搜索路径（根据你的安装情况选择一种）
# 情况1：已经执行 autobuild.sh 安装到系统
INC_DIR = /usr/include/mymuduo
LIB_DIR = /usr/lib

# 情况2：未安装，使用项目内的库（注释掉情况1，取消注释下面两行）
# INC_DIR = ../include
# LIB_DIR = ../lib

# 库名称
LIB_NAME = my_muduo

# 编译规则
all: $(TARGET)

# 直接编译链接，不生成 .o 中间文件
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $^ -L$(LIB_DIR) -l$(LIB_NAME) $(LDFLAGS) -o $@

# 清理
clean:
	rm -f $(TARGET)

# 运行
run: $(TARGET)
	./$(TARGET)

# 帮助
help:
	@echo "使用方法:"
	@echo "  make         - 编译 echo_server"
	@echo "  make clean   - 清理编译产物"
	@echo "  make run     - 编译并运行"
	@echo ""
	@echo "当前配置:"
	@echo "  头文件路径: $(INC_DIR)"
	@echo "  库文件路径: $(LIB_DIR)"
	@echo "  库名称: lib$(LIB_NAME).so"

.PHONY: all clean run help