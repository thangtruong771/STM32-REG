CC = arm-none-eabi-gcc
BUILD_FLAG = -mcpu=cortex-m4 -std=gnu11 -g

ALL:
	$(CC) main.c -c $(BUILD_FLAG) -o main.o
	$(CC) startup_stm32f411vetx.s -c $(BUILD_FLAG) -o startup.o
	$(CC) main.o startup.o $(BUILD_FLAG) -T"STM32F411VETX_FLASH.ld" -Wl,-Map="file.map" -Wl,--gc-sections -static -o fw.elf
	arm-none-eabi-objcopy -O ihex fw.elf fw.hex

load:
	"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -SE 0 2 -P fw.hex -V -Run
