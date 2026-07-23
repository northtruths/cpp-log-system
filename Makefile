# ============================================================
#  项目配置
# ============================================================
TARGET           := test

# 编译器与标准
CXX              := g++
CXXFLAGS         := -std=c++17 -Wall -Wextra -g -O0
LDFLAGS          := -lpthread

# 目录结构
SRC_DIR          := src
TEST_DIR         := tests
BUILD_DIR        := build
BIN_DIR          := bin
INC_DIR          := $(SRC_DIR)

# ============================================================
#  源文件收集
# ============================================================
CORE_SRCS        := $(wildcard $(SRC_DIR)/*.cpp)
TEST_SRCS        := $(wildcard $(TEST_DIR)/main.cpp)

# 目标文件
CORE_OBJS        := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRCS))
TEST_OBJS        := $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(TEST_SRCS))

# ============================================================
#  构建
# ============================================================
.PHONY: all
all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(CORE_OBJS) $(TEST_OBJS) | $(BIN_DIR)
	@echo "  [LD]  $@"
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "  编译完成: $@"

# 编译 src
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "  [CC]  $<"
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# 编译 tests
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "  [CC]  $<"
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# 目录
$(BIN_DIR) $(BUILD_DIR):
	@mkdir -p $@

# ============================================================
#  辅助
# ============================================================
.PHONY: clean run

clean:
	@echo "  清理..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR) 
	@rm -rf logs

run: all
	@./$(BIN_DIR)/$(TARGET)