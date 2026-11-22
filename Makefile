DEVKITXENON ?= /usr/local/xenon
PATH := $(DEVKITXENON)/bin:$(DEVKITXENON)/usr/bin:$(PATH)
export PATH

CC := xenon-gcc
CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Werror

BUILD_DIR := build
OBJECT := $(BUILD_DIR)/main.o

.PHONY: all clean toolchain-info

all: $(OBJECT)

$(BUILD_DIR):
	mkdir -p $@

$(OBJECT): src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

toolchain-info:
	command -v $(CC)
	$(CC) --version

clean:
	rm -rf $(BUILD_DIR)
