TARGET = main

BUILD_DIR = build
SRC_DIR   = .
STARTUP   = startup_stm32f411vetx.s
LINKER    = STM32F411VETX_FLASH.ld

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

MCUFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CSTD     = -std=gnu11
CFLAGS   = $(MCUFLAGS) $(CSTD) -g3 -O0 -Wall -Wextra -Werror -pedantic -Wconversion \
           -ffunction-sections -fdata-sections -fstack-usage \
           -DDEBUG -DSTM32F411VETx -DSTM32 -DSTM32F4 -DSTM32F411E_DISCO \
           --specs=nano.specs

LDFLAGS  = -T $(LINKER) -Wl,--gc-sections

C_SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS   := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES)) \
             $(BUILD_DIR)/startup_stm32f411xe.o

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/startup_stm32f411xe.o: $(STARTUP) | $(BUILD_DIR)
	$(CC) $(MCUFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Built $@"

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	@echo "Created binary: $@"

flash:
	STM32_Programmer_CLI -c port=SWD -w build/main.elf -rst
	@echo "Programming Completed"

clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

.PHONY: all clean flash