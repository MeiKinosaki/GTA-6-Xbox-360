XSDK_ROOT ?= $(HOME)/xsdk
XSDK_TOOLCHAIN ?= $(HOME)/xsdk-gcc
BUILD_DIR ?= build/xsdk-title
CMAKE ?= cmake

.PHONY: all configure clean

all: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel
	test -s $(BUILD_DIR)/gta6.xex
	python3 -c 'from pathlib import Path; p=Path("$(BUILD_DIR)/gta6.xex"); d=p.read_bytes(); assert d[:4] == b"XEX2", "invalid XEX2 header"; Path("default.xex").write_bytes(d); print("default.xex:", len(d), "bytes")'

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(XSDK_ROOT)/cmake/xsdk.cmake \
		-DXSDK_TOOLCHAIN=$(XSDK_TOOLCHAIN) \
		-DCMAKE_BUILD_TYPE=Release

clean:
	rm -rf $(BUILD_DIR) default.xex
