DEVKITXENON ?= /usr/local/xenon
PATH := $(DEVKITXENON)/bin:$(DEVKITXENON)/usr/bin:$(PATH)
export PATH

CC := xenon-gcc
OBJCOPY := xenon-objcopy
READELF := powerpc-linux-gnu-readelf
CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Werror \
	-mcpu=cell -m32 -maltivec -fno-pic -mpowerpc64 -mhard-float -fwrapv
LDFLAGS := -nostdlib -Wl,-n -Wl,-u,_start -Wl,-T,$(DEVKITXENON)/app.lds
LIBPATHS := -L$(DEVKITXENON)/usr/lib -L$(DEVKITXENON)/xenon/lib/32
LIBS := -lxenon -lc -lgcc

BUILD_DIR := build
OBJECT := $(BUILD_DIR)/main.o
ELF := Xenon.elf
ELF32 := Xenon.elf32

.PHONY: all clean toolchain-info verify-elf

all: $(OBJECT)

$(BUILD_DIR):
	mkdir -p $@

$(OBJECT): src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJECT)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LIBPATHS) $(LIBS) -o $@

$(ELF32): $(ELF)
	$(OBJCOPY) -O elf32-powerpc --adjust-vma 0x80000000 $< $@

verify-elf: $(ELF) $(ELF32)
	file $(ELF) $(ELF32)
	$(DEVKITXENON)/bin/xenon-objdump -f $(ELF)

toolchain-info:
	command -v $(CC)
	$(CC) --version
	command -v $(OBJCOPY)

clean:
	rm -rf $(BUILD_DIR) $(ELF) $(ELF32)
