CC = arm-none-eabi-gcc
BUILD_DIR = build
KERNEL_DIR = FreeRTOS-Kernel
BUILD_FLAG = -specs=nano.specs -specs=nosys.specs -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -std=gnu11 -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/portable/GCC/ARM_CM4F
LINK_FLAG = -T"STM32F411VETX_FLASH.ld" -Wl,-Map="build/freeRTOS_proj.map" -Wl,--gc-sections -static

All:
	$(CC) -c main.c $(BUILD_FLAG) -o $(BUILD_DIR)/main.o
	$(CC) -c startup_stm32f411vetx.s $(BUILD_FLAG) -o $(BUILD_DIR)/startup.o
	$(CC) -c $(KERNEL_DIR)/croutine.c $(BUILD_FLAG) -o $(BUILD_DIR)/croutine.o
	$(CC) -c $(KERNEL_DIR)/event_groups.c $(BUILD_FLAG) -o $(BUILD_DIR)/event_groups.o
	$(CC) -c $(KERNEL_DIR)/list.c $(BUILD_FLAG) -o $(BUILD_DIR)/list.o
	$(CC) -c $(KERNEL_DIR)/queue.c $(BUILD_FLAG) -o $(BUILD_DIR)/queue.o
	$(CC) -c $(KERNEL_DIR)/stream_buffer.c $(BUILD_FLAG) -o $(BUILD_DIR)/stream_buffer.o
	$(CC) -c $(KERNEL_DIR)/tasks.c $(BUILD_FLAG) -o $(BUILD_DIR)/tasks.o
	$(CC) -c $(KERNEL_DIR)/timers.c $(BUILD_FLAG) -o $(BUILD_DIR)/timers.o
	$(CC) -c $(KERNEL_DIR)/portable/GCC/ARM_CM4F/port.c $(BUILD_FLAG) -o $(BUILD_DIR)/port.o
	$(CC) -c $(KERNEL_DIR)/portable/MemMang/heap_4.c $(BUILD_FLAG) -o $(BUILD_DIR)/heap_4.o
	$(CC) $(BUILD_DIR)/*.o $(BUILD_FLAG) $(LINK_FLAG) -o $(BUILD_DIR)/freeRTOS_proj.elf
	arm-none-eabi-objcopy -O ihex $(BUILD_DIR)/freeRTOS_proj.elf $(BUILD_DIR)/freeRTOS_proj.hex
load:
	"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -SE 0 3 -P $(BUILD_DIR)/freeRTOS_proj.hex -V -Run