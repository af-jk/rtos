CFLAGS  ?=  -W -Wall -Wextra -Wundef -Wshadow -Wdouble-promotion \
            -Wformat-truncation -fno-common -Wconversion \
            -g3 -O0 -ffunction-sections -fdata-sections \
            -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
LDFLAGS ?= -Wl,-Map=build/cm4.map -T./cm4.ld -nostartfiles --specs nosys.specs -lc -lgcc

_dummy := $(shell mkdir -p build)
BUILDDIR := build

CSOURCEDIR := src
ASMSOURCEDIR := $(CSOURCEDIR)/asm

SOURCES := $(shell find $(CSOURCEDIR) -name '*.c') 
ASMSOURCES := $(shell find $(ASMSOURCEDIR) -name '*.s')

INCDIR := ./inc 
INCLUDES := $(addprefix -I, $(INCDIR))

COBJECTS := $(patsubst $(CSOURCEDIR)/%.c,$(BUILDDIR)/%.o,$(filter %.c,$(SOURCES)))
ASMOBJECTS += $(patsubst $(ASMSOURCEDIR)/%.s,$(BUILDDIR)/%.o,$(filter %.s,$(ASMSOURCES)))

OBJECTS := $(ASMOBJECTS) $(COBJECTS)

build: firmware.bin

firmware.elf: $(OBJECTS)
	arm-none-eabi-gcc $(OBJECTS) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary $< $@

flash: firmware.bin
	st-flash --reset write $< 0x8000000

$(BUILDDIR)/%.o: $(ASMSOURCEDIR)/%.s
	arm-none-eabi-as -c $< -o $@

$(BUILDDIR)/%.o: $(CSOURCEDIR)/%.c 
	arm-none-eabi-gcc -c $(CFLAGS) $(INCLUDES) $< -o $@

.PHONY: clean
clean:
	rm -f firmware.*
	rm -rf $(BUILDDIR)