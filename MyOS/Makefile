# Makefile for MyOS
# Build system untuk sistem operasi MyOS

# Compiler dan tools
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld
OBJCOPY = i686-elf-objcopy
QEMU = qemu-system-i386

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Directories
BOOT_DIR = boot
KERNEL_DIR = kernel
USER_DIR = userspace
BUILD_DIR = build
ISO_DIR = iso

# Source files
BOOT_ASM = $(BOOT_DIR)/bootloader.asm
KERNEL_SOURCES = $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_ASM = $(shell find $(KERNEL_DIR) -name "*.asm")
USER_SOURCES = $(shell find $(USER_DIR) -name "*.c")

# Object files
KERNEL_OBJECTS = $(KERNEL_SOURCES:%.c=$(BUILD_DIR)/%.o) $(KERNEL_ASM:%.asm=$(BUILD_DIR)/%.o)
USER_OBJECTS = $(USER_SOURCES:%.c=$(BUILD_DIR)/%.o)

# Output files
BOOTLOADER = $(BUILD_DIR)/bootloader.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/myos.img
ISO_IMAGE = $(BUILD_DIR)/myos.iso

# Default target
all: $(OS_IMAGE)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/core
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/memory
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/process
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/drivers
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/filesystem
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/lib/libc
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/lib/libgui
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/shell
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/gui/desktop
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/gui/window_manager
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/gui/applications
	mkdir -p $(BUILD_DIR)/$(USER_DIR)/utilities

# Build bootloader
$(BOOTLOADER): $(BOOT_ASM) | $(BUILD_DIR)
	$(AS) -f bin $< -o $@

# Build kernel object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(KERNEL_OBJECTS) linker.ld | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $(KERNEL_OBJECTS) -o $@

# Create OS disk image
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL_BIN) | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=1M count=10
	dd if=$(BOOTLOADER) of=$@ conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ seek=1 conv=notrunc

# Create ISO image
$(ISO_IMAGE): $(KERNEL_BIN) | $(BUILD_DIR)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	echo 'menuentry "MyOS" {' > $(ISO_DIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISO_DIR)

# Run in QEMU
run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -m 128M

run-iso: $(ISO_IMAGE)
	$(QEMU) -cdrom $(ISO_IMAGE) -m 128M

# Debug in QEMU
debug: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -m 128M -s -S

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(ISO_DIR)

# Install cross-compiler (Ubuntu/Debian)
install-deps:
	sudo apt update
	sudo apt install build-essential nasm qemu-system-x86 grub-pc-bin xorriso mtools
	
	# Download and build cross-compiler
	mkdir -p tools/cross-compiler
	cd tools/cross-compiler && \
	wget https://ftp.gnu.org/gnu/binutils/binutils-2.37.tar.gz && \
	wget https://ftp.gnu.org/gnu/gcc/gcc-11.2.0/gcc-11.2.0.tar.gz && \
	tar -xzf binutils-2.37.tar.gz && \
	tar -xzf gcc-11.2.0.tar.gz && \
	mkdir build-binutils build-gcc && \
	cd build-binutils && \
	../binutils-2.37/configure --target=i686-elf --prefix=$(HOME)/opt/cross --disable-nls --disable-werror && \
	make && make install && \
	cd ../build-gcc && \
	../gcc-11.2.0/configure --target=i686-elf --prefix=$(HOME)/opt/cross --disable-nls --enable-languages=c,c++ --without-headers && \
	make all-gcc && make all-target-libgcc && \
	make install-gcc && make install-target-libgcc

# Create linker script
linker.ld:
	echo 'ENTRY(_start)' > linker.ld
	echo 'SECTIONS {' >> linker.ld
	echo '    . = 0x100000;' >> linker.ld
	echo '    .text : { *(.text) }' >> linker.ld
	echo '    .data : { *(.data) }' >> linker.ld
	echo '    .bss : { *(.bss) }' >> linker.ld
	echo '}' >> linker.ld

# Create boot configuration
boot-config:
	mkdir -p config
	echo '# MyOS System Configuration' > config/system.conf
	echo 'kernel_version=1.0' >> config/system.conf
	echo 'memory_size=128MB' >> config/system.conf
	echo 'gui_enabled=true' >> config/system.conf
	echo 'network_enabled=true' >> config/system.conf

# Testing targets
test-boot: $(BOOTLOADER)
	$(QEMU) -drive format=raw,file=$(BOOTLOADER) -m 128M

test-kernel: $(KERNEL_BIN)
	$(QEMU) -kernel $(KERNEL_BIN) -m 128M

# Create startup scripts
scripts:
	mkdir -p tools
	echo '#!/bin/bash' > tools/build.sh
	echo 'echo "Building MyOS..."' >> tools/build.sh
	echo 'make clean' >> tools/build.sh
	echo 'make all' >> tools/build.sh
	echo 'echo "Build complete!"' >> tools/build.sh
	chmod +x tools/build.sh
	
	echo '#!/bin/bash' > tools/qemu-test.sh
	echo 'echo "Starting MyOS in QEMU..."' >> tools/qemu-test.sh
	echo 'make run' >> tools/qemu-test.sh
	chmod +x tools/qemu-test.sh
	
	echo '#!/bin/bash' > tools/disk-image.sh
	echo 'echo "Creating disk image..."' >> tools/disk-image.sh
	echo 'dd if=/dev/zero of=build/disk.img bs=1M count=100' >> tools/disk-image.sh
	echo 'mkfs.ext2 build/disk.img' >> tools/disk-image.sh
	echo 'echo "Disk image created!"' >> tools/disk-image.sh
	chmod +x tools/disk-image.sh

# Documentation targets
docs:
	mkdir -p docs
	echo '# MyOS Architecture' > docs/architecture.md
	echo '' >> docs/architecture.md
	echo '## Overview' >> docs/architecture.md
	echo 'MyOS is a simple operating system built from scratch with the following components:' >> docs/architecture.md
	echo '' >> docs/architecture.md
	echo '### Kernel' >> docs/architecture.md
	echo '- Memory management (physical and virtual)' >> docs/architecture.md
	echo '- Process scheduling' >> docs/architecture.md
	echo '- Interrupt handling' >> docs/architecture.md
	echo '- Device drivers' >> docs/architecture.md
	echo '- File system' >> docs/architecture.md
	echo '' >> docs/architecture.md
	echo '### User Space' >> docs/architecture.md
	echo '- GUI desktop environment' >> docs/architecture.md
	echo '- Window manager' >> docs/architecture.md
	echo '- Applications' >> docs/architecture.md
	echo '- Shell' >> docs/architecture.md
	echo '- System utilities' >> docs/architecture.md
	
	echo '# Building MyOS' > docs/building.md
	echo '' >> docs/building.md
	echo '## Prerequisites' >> docs/building.md
	echo '- i686-elf cross-compiler' >> docs/building.md
	echo '- NASM assembler' >> docs/building.md
	echo '- QEMU emulator' >> docs/building.md
	echo '- Make build system' >> docs/building.md
	echo '' >> docs/building.md
	echo '## Build Commands' >> docs/building.md
	echo '```bash' >> docs/building.md
	echo 'make install-deps  # Install dependencies' >> docs/building.md
	echo 'make all          # Build everything' >> docs/building.md
	echo 'make run          # Run in QEMU' >> docs/building.md
	echo 'make clean        # Clean build files' >> docs/building.md
	echo '```' >> docs/building.md

# Help target
help:
	@echo "MyOS Build System"
	@echo "================="
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Build complete OS image"
	@echo "  run           - Run OS in QEMU"
	@echo "  run-iso       - Run ISO image in QEMU"
	@echo "  debug         - Run OS in QEMU with debugging"
	@echo "  clean         - Remove all build files"
	@echo "  install-deps  - Install build dependencies"
	@echo "  test-boot     - Test bootloader only"
	@echo "  test-kernel   - Test kernel only"
	@echo "  scripts       - Create helper scripts"
	@echo "  docs          - Generate documentation"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Build outputs:"
	@echo "  $(OS_IMAGE)   - Complete OS disk image"
	@echo "  $(ISO_IMAGE)  - Bootable ISO image"
	@echo "  $(KERNEL_BIN) - Kernel binary"
	@echo "  $(BOOTLOADER) - Bootloader binary"

# Phony targets
.PHONY: all run run-iso debug clean install-deps test-boot test-kernel scripts docs help boot-config

# Dependencies
$(BUILD_DIR)/kernel/core/kernel.o: kernel/include/kernel.h kernel/include/memory.h kernel/include/graphics.h
$(BUILD_DIR)/kernel/memory/memory.o: kernel/include/memory.h kernel/include/kernel.h
$(BUILD_DIR)/kernel/filesystem/vfs.o: kernel/include/filesystem.h kernel/include/memory.h
$(BUILD_DIR)/userspace/gui/desktop/desktop.o: userspace/lib/libgui/graphics.h userspace/lib/libgui/window.h