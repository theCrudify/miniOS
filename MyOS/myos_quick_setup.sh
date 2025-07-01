#!/bin/bash
# MyOS Quick Setup Script
# Run this to get your development environment ready

set -e

echo "🚀 MyOS Quick Setup Script"
echo "=========================="

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() { echo -e "${GREEN}[INFO]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARN]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_step() { echo -e "${BLUE}[STEP]${NC} $1"; }

# Check OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    print_error "Please use WSL2 on Windows instead of native Windows"
    exit 1
else
    print_error "Unsupported OS: $OSTYPE"
    exit 1
fi

print_status "Detected OS: $OS"

# Install dependencies
print_step "Installing dependencies..."

if [[ "$OS" == "linux" ]]; then
    # Check if apt is available
    if command -v apt &> /dev/null; then
        sudo apt update
        sudo apt install -y build-essential nasm qemu-system-x86 wget curl
        sudo apt install -y grub-pc-bin xorriso mtools
    elif command -v yum &> /dev/null; then
        sudo yum groupinstall -y "Development Tools"
        sudo yum install -y nasm qemu-system-x86 wget curl
    else
        print_error "Package manager not found. Please install manually."
        exit 1
    fi
elif [[ "$OS" == "macos" ]]; then
    if ! command -v brew &> /dev/null; then
        print_step "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    brew install nasm qemu wget
fi

print_status "Dependencies installed"

# Check for cross-compiler
print_step "Checking cross-compiler..."

if command -v i686-elf-gcc &> /dev/null; then
    print_status "Cross-compiler already installed"
else
    print_warning "Cross-compiler not found. Installing..."
    
    # Create cross-compiler build script
    cat > build_cross_compiler.sh << 'EOF'
#!/bin/bash
set -e

export TARGET=i686-elf
export PREFIX="$HOME/opt/cross"
export PATH="$PREFIX/bin:$PATH"

mkdir -p $HOME/src
cd $HOME/src

echo "Downloading binutils..."
wget -nc https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
echo "Downloading GCC..."
wget -nc https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz

echo "Extracting..."
tar -xf binutils-2.40.tar.gz
tar -xf gcc-13.2.0.tar.gz

echo "Building binutils..."
mkdir -p build-binutils
cd build-binutils
../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

echo "Building GCC..."
cd ..
mkdir -p build-gcc
cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

echo "Cross-compiler built successfully!"
EOF

    chmod +x build_cross_compiler.sh
    print_warning "Building cross-compiler... This will take 15-30 minutes."
    ./build_cross_compiler.sh
    
    # Add to PATH
    echo 'export PATH="$HOME/opt/cross/bin:$PATH"' >> ~/.bashrc
    export PATH="$HOME/opt/cross/bin:$PATH"
fi

# Verify tools
print_step "Verifying tools..."

TOOLS=("i686-elf-gcc" "nasm" "qemu-system-i386" "make")
for tool in "${TOOLS[@]}"; do
    if command -v "$tool" &> /dev/null; then
        print_status "$tool: $(which $tool)"
    else
        print_error "$tool not found!"
        exit 1
    fi
done

# Fix MyOS project
print_step "Setting up MyOS project..."

# Create missing files
mkdir -p MyOS/build

# Create working linker script
cat > MyOS/linker.ld << 'EOF'
ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    .text ALIGN(4K) :
    {
        *(.text)
    }
    
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }
    
    .data ALIGN(4K) :
    {
        *(.data)
    }
    
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
    
    end = .;
}
EOF

# Create minimal working bootloader
cat > MyOS/boot/bootloader.asm << 'EOF'
[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Clear screen
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    ; Print message
    mov si, msg
    call print_string
    
    ; Load kernel (simplified)
    mov ah, 0x02
    mov al, 10
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, 0x80
    mov bx, 0x1000
    int 0x13
    
    jc disk_error
    
    ; Jump to kernel
    jmp 0x1000
    
disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

msg db 'MyOS Bootloader v1.0', 0x0D, 0x0A, 'Loading kernel...', 0x0D, 0x0A, 0
error_msg db 'Disk read error!', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
EOF

# Create minimal kernel entry point
cat > MyOS/kernel/core/boot_entry.asm << 'EOF'
[BITS 32]

extern kernel_main

global _start
_start:
    mov esp, 0x90000
    cld
    call kernel_main
.halt:
    hlt
    jmp .halt
EOF

# Create minimal kernel
cat > MyOS/kernel/core/kernel.c << 'EOF'
// Minimal working kernel
void print_string(const char* str, int x, int y, unsigned char color) {
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    int i = 0;
    while(str[i]) {
        vga[(y * 80 + x + i)] = (color << 8) | str[i];
        i++;
    }
}

void kernel_main(void) {
    // Clear screen
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    for(int i = 0; i < 80 * 25; i++) {
        vga[i] = (0x07 << 8) | ' ';
    }
    
    // Print welcome message
    print_string("Welcome to MyOS!", 0, 0, 0x0F);
    print_string("Kernel loaded successfully!", 0, 1, 0x0A);
    print_string("System is running...", 0, 2, 0x0E);
    
    // Infinite loop
    while(1) {
        asm volatile("hlt");
    }
}
EOF

# Create simplified Makefile for testing
cat > MyOS/Makefile.simple << 'EOF'
CC = i686-elf-gcc
AS = nasm
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

all: myos.img

myos.img: bootloader.bin kernel.bin
	dd if=/dev/zero of=myos.img bs=1M count=10
	dd if=bootloader.bin of=myos.img conv=notrunc
	dd if=kernel.bin of=myos.img seek=1 conv=notrunc

bootloader.bin: boot/bootloader.asm
	$(AS) -f bin boot/bootloader.asm -o bootloader.bin

kernel.bin: kernel/core/boot_entry.o kernel/core/kernel.o
	$(CC) $(LDFLAGS) kernel/core/boot_entry.o kernel/core/kernel.o -o kernel.bin

kernel/core/boot_entry.o: kernel/core/boot_entry.asm
	$(AS) -f elf32 kernel/core/boot_entry.asm -o kernel/core/boot_entry.o

kernel/core/kernel.o: kernel/core/kernel.c
	$(CC) $(CFLAGS) -c kernel/core/kernel.c -o kernel/core/kernel.o

run: myos.img
	qemu-system-i386 -drive format=raw,file=myos.img -m 128M

debug: myos.img
	qemu-system-i386 -drive format=raw,file=myos.img -m 128M -s -S

clean:
	rm -f *.bin *.o *.img kernel/core/*.o

.PHONY: all run debug clean
EOF

print_status "MyOS project setup complete"

# Test build
print_step "Testing build..."
cd MyOS

make -f Makefile.simple clean
make -f Makefile.simple all

if [ -f "myos.img" ]; then
    print_status "Build successful! myos.img created"
else
    print_error "Build failed!"
    exit 1
fi

# Create run script
cat > run_myos.sh << 'EOF'
#!/bin/bash
echo "Starting MyOS in QEMU..."
echo "Press Ctrl+Alt+G to release mouse cursor"
echo "Press Ctrl+Alt+Q to quit QEMU"
echo ""
make -f Makefile.simple run
EOF

chmod +x run_myos.sh

print_status "Setup complete!"

echo ""
echo "================================"
echo -e "${GREEN}🎉 MyOS Setup Complete! 🎉${NC}"
echo "================================"
echo ""
echo "📁 Location: $(pwd)"
echo "🔧 Cross-compiler: $(which i686-elf-gcc)"
echo "🖥️  QEMU: $(which qemu-system-i386)"
echo ""
echo "🚀 To run MyOS:"
echo "   cd MyOS"
echo "   ./run_myos.sh"
echo ""
echo "🔧 To rebuild:"
echo "   make -f Makefile.simple clean"
echo "   make -f Makefile.simple all"
echo ""
echo "Your OS is ready to run! 🚀"