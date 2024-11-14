CFLAGS  ?=  -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -O0 -ffunction-sections -fdata-sections \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
LDFLAGS ?= -T./cm4.ld -nostdlib -nostartfiles --specs nano.specs -lc -lgcc

SOURCEDIR := src
SOURCES := $(shell find $(SOURCEDIR) -name '*.c') 
SOURCES += $(shell find $(SOURCEDIR) -name '*.s')

INCDIR := ./inc 
INCLUDES := $(addprefix -I, $(INCDIR))

build: firmware.bin

firmware.elf: $(SOURCES)
	arm-none-eabi-gcc $(SOURCES) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: firmware.bin
	st-flash --reset write $< 0x8000000

.PHONY: clean
clean:
	rm -f firmware.*