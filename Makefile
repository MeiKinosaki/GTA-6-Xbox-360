DEVKITXENON ?= /usr/local/xenon
PATH := $(DEVKITXENON)/bin:$(DEVKITXENON)/usr/bin:$(PATH)
export PATH

CC := xenon-gcc
LD := xenon-ld
OBJCOPY := xenon-objcopy
OBJDUMP := xenon-objdump
CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Werror \
	-mcpu=cell -mtune=cell -m32 -maltivec -fno-pic -mpowerpc64 \
	-mhard-float -fwrapv -ffreestanding -fno-builtin -fno-stack-protector
ASFLAGS := -m32 -mcpu=cell -maltivec -fno-pic -mpowerpc64
LDFLAGS := -n -T $(DEVKITXENON)/app.lds

BUILD_DIR := build
MAIN_OBJECT := $(BUILD_DIR)/main.o
START_OBJECT := $(BUILD_DIR)/start.o
OBJECTS := $(START_OBJECT) $(MAIN_OBJECT)
ELF := Xenon.elf
ELF32 := Xenon.elf32

.PHONY: all clean toolchain-info verify-elf

all: $(ELF) $(ELF32)

$(BUILD_DIR):
	mkdir -p $@

$(MAIN_OBJECT): src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(START_OBJECT): src/start.S | $(BUILD_DIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(ELF): $(OBJECTS)
	test -f $(DEVKITXENON)/app.lds
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(ELF32): $(ELF)
	$(OBJCOPY) -O elf32-powerpc --adjust-vma 0x80000000 $< $@

verify-elf: all
	file $(ELF) $(ELF32)
	$(OBJDUMP) -f $(ELF)
	$(OBJDUMP) -h $(ELF)

toolchain-info:
	command -v $(CC)
	$(CC) --version
	command -v $(LD)
	command -v $(OBJCOPY)
	command -v $(OBJDUMP)

clean:
	rm -rf $(BUILD_DIR) $(ELF) $(ELF32)
