# 编译器设置
CROSS_COMPILE ?= riscv32-unknown-elf-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# 目录设置
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/objs
DEP_DIR = $(BUILD_DIR)/deps

# 编译标志
ARCH = -march=rv32g -mabi=ilp32
OPT_FLAGS = -O0 -g3
CFLAGS = $(ARCH) $(OPT_FLAGS) -MMD -MP -MF $(DEP_DIR)/$*.d
LDFLAGS = $(ARCH) \
-static -nostartfiles \
-Wl,--no-warn-rwx-segments \
-T $(SRC_DIR)/link.ld \
-Wl,-Map=$(BUILD_DIR)/$(TARGET).map 

# 源文件和目标文件
SRCS_C = $(wildcard $(SRC_DIR)/*.c)
SRCS_ASM = $(wildcard $(SRC_DIR)/*.S)
OBJS = $(SRCS_C:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) $(SRCS_ASM:$(SRC_DIR)/%.S=$(OBJ_DIR)/%.o)
DEPS = $(SRCS_C:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

# 目标文件
TARGET = rv32_hello
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf
TARGET_BIN = $(BUILD_DIR)/$(TARGET).bin
TARGET_DUMP = $(BUILD_DIR)/$(TARGET).dump

.PHONY: all clean run debug

all: $(TARGET_ELF) $(TARGET_BIN) $(TARGET_DUMP)

# 创建必要的目录
$(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR):
	mkdir -p $@

# 编译C文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 链接
$(TARGET_ELF): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

# 生成二进制文件
$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary $< $@

# 生成反汇编文件
$(TARGET_DUMP): $(TARGET_ELF)
	$(OBJDUMP) -D $< > $@

# 在QEMU上运行
run: $(TARGET_ELF)
	qemu-system-riscv32 -machine virt -nographic -no-reboot -bios none -kernel $(TARGET_ELF)

# 在QEMU上调试
debug: $(TARGET_ELF)
	qemu-system-riscv32 -machine virt -nographic -no-reboot -bios none -kernel $(TARGET_ELF) -S -s

# 清理
clean:
	rm -rf $(BUILD_DIR)

# 包含自动生成的依赖文件
-include $(DEPS)