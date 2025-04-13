CFLAGS  ?=  -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -O0 -ffunction-sections -fdata-sections \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
LDFLAGS ?= -Wl,-Map=build/cm4.map -T./cm4.ld -nostartfiles --specs nosys.specs -lc -lgcc

_dummy := $(shell mkdir -p build)
BUILDDIR := build

SOURCEDIR := src

SOURCES := $(shell find $(SOURCEDIR) -name '*.c') 
SOURCES += $(shell find $(SOURCEDIR) -name '*.s')

INCDIR := ./inc 
INCLUDES := $(addprefix -I, $(INCDIR))

COBJECTS := $(patsubst $(SOURCEDIR)/%.c,$(BUILDDIR)/%.o,$(filter %.c,$(SOURCES)))
ASMOBJECTS += $(patsubst $(SOURCEDIR)/%.s,$(BUILDDIR)/%.o,$(filter %.s,$(SOURCES)))

OBJECTS := $(ASMOBJECTS) $(COBJECTS)

build: firmware.bin

firmware.elf: $(OBJECTS)
	arm-none-eabi-gcc $(OBJECTS) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: firmware.bin
	st-flash --reset write $< 0x8000000

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.s
	arm-none-eabi-as -c $< -o $@

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.c 
	arm-none-eabi-gcc -c $(CFLAGS) $(INCLUDES) $< -o $@

.PHONY: clean
clean:
	rm -f firmware.*
	rm -rf $(BUILDDIR)