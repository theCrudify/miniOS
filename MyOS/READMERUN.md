# MyOS Complete Setup Guide 🚀

## 📋 What You Need

Your MyOS project requires specific tools to compile and run. Here's everything you need to install:

### Required Tools
- **Linux Environment** (WSL2 if on Windows)
- **Cross-compiler** (i686-elf-gcc)
- **NASM Assembler**
- **QEMU Emulator**
- **Build tools** (make, binutils)

## 🖥️ Platform-Specific Setup

### Option 1: Windows with WSL2 (Recommended)

1. **Install WSL2**:
   ```powershell
   # Run in PowerShell as Administrator
   wsl --install -d Ubuntu
   ```

2. **Open Ubuntu WSL2** and run:
   ```bash
   # Update system
   sudo apt update && sudo apt upgrade -y
   
   # Install essential tools
   sudo apt install -y build-essential nasm qemu-system-x86 
   sudo apt install -y grub-pc-bin xorriso mtools wget curl
   ```

### Option 2: Native Linux (Ubuntu/Debian)

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install required packages
sudo apt install -y build-essential nasm qemu-system-x86
sudo apt install -y grub-pc-bin xorriso mtools wget curl git
```

### Option 3: macOS

```bash
# Install Homebrew if not installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required tools
brew install nasm qemu wget
brew install i686-elf-gcc  # May need additional setup
```

## 🔧 Cross-Compiler Setup

The most critical component is the cross-compiler. Here's how to build it:

### Automated Installation Script

Save this as `install_cross_compiler.sh`:

```bash
#!/bin/bash
set -e

echo "🔧 Building Cross-Compiler for MyOS..."

# Configuration
export TARGET=i686-elf
export PREFIX="$HOME/opt/cross"
export PATH="$PREFIX/bin:$PATH"

# Create directories
mkdir -p $HOME/src
cd $HOME/src

# Download sources
echo "📥 Downloading sources..."
wget -nc https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
wget -nc https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz

# Extract
echo "📦 Extracting..."
tar -xf binutils-2.40.tar.gz
tar -xf gcc-13.2.0.tar.gz

# Build Binutils
echo "🔨 Building Binutils..."
mkdir -p build-binutils
cd build-binutils
../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Build GCC
echo "🔨 Building GCC..."
cd ..
mkdir -p build-gcc
cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

echo "✅ Cross-compiler installed successfully!"
echo "Add this to your ~/.bashrc:"
echo "export PATH=\"\$HOME/opt/cross/bin:\$PATH\""
```

Run it:
```bash
chmod +x install_cross_compiler.sh
./install_cross_compiler.sh
```

Add to your shell profile:
```bash
echo 'export PATH="$HOME/opt/cross/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

## 🚀 Building and Running MyOS

### Step 1: Prepare the Project

1. **Clone/Download your MyOS project**
2. **Navigate to project directory**:
   ```bash
   cd MyOS
   ```

3. **Verify tools**:
   ```bash
   # Check if cross-compiler is working
   i686-elf-gcc --version
   nasm --version
   qemu-system-i386 --version
   ```

### Step 2: Fix Common Issues

Your project has some missing files. Create them:

**Create missing linker script** (`linker.ld`):
```ld
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
```

**Fix Makefile paths** - ensure all source files exist or modify the Makefile to only build existing files.

### Step 3: Build the OS

```bash
# Clean any previous builds
make clean

# Build everything
make all

# If there are missing files, start with basic components:
make bootloader  # Build just bootloader first
make kernel      # Then kernel
```

### Step 4: Run in QEMU

```bash
# Run the OS
make run

# For debugging
make debug
```

## 🐛 Troubleshooting Common Issues

### Issue 1: Cross-compiler not found
```bash
# Check if it's in PATH
which i686-elf-gcc

# If not found, add to PATH:
export PATH="$HOME/opt/cross/bin:$PATH"
```

### Issue 2: Missing source files
Many of your source files are empty. You'll need to implement them or start with a minimal version:

**Minimal kernel.c**:
```c
void kernel_main(void) {
    volatile unsigned short* vga = (volatile unsigned short*)0xB8000;
    const char* message = "Hello MyOS!";
    
    // Clear screen
    for(int i = 0; i < 80 * 25; i++) {
        vga[i] = (0x07 << 8) | ' ';
    }
    
    // Display message
    for(int i = 0; message[i] != '\0'; i++) {
        vga[i] = (0x07 << 8) | message[i];
    }
    
    while(1) asm("hlt");
}
```

### Issue 3: Build errors
```bash
# Check what files actually exist
find . -name "*.c" -o -name "*.asm" | head -10

# Build incrementally
make bootloader
make kernel
```

### Issue 4: QEMU doesn't start
```bash
# Try different QEMU options
qemu-system-i386 -drive format=raw,file=build/myos.img -m 128M -display gtk

# Or with serial output for debugging
qemu-system-i386 -drive format=raw,file=build/myos.img -m 128M -serial stdio
```

## 📁 Minimal Working Version

If you want to start with something that definitely works, here's a minimal approach:

### 1. Simple Bootloader (`boot/simple_boot.asm`):
```assembly
[BITS 16]
[ORG 0x7C00]

start:
    mov ax, 0x07C0
    mov ds, ax
    mov si, msg
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

msg db 'MyOS Bootloader', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
```

### 2. Simple Makefile:
```makefile
BOOTLOADER = boot/simple_boot.asm

all: myos.img

myos.img: boot.bin
	dd if=/dev/zero of=myos.img bs=1M count=1
	dd if=boot.bin of=myos.img conv=notrunc

boot.bin: $(BOOTLOADER)
	nasm -f bin $(BOOTLOADER) -o boot.bin

run: myos.img
	qemu-system-i386 -drive format=raw,file=myos.img

clean:
	rm -f *.bin *.img

.PHONY: all run clean
```

## 🎯 Quick Start Commands

Once everything is set up:

```bash
# 1. Install tools (Ubuntu/Debian)
sudo apt install build-essential nasm qemu-system-x86

# 2. Build cross-compiler (run the script above)
./install_cross_compiler.sh

# 3. Add to PATH
export PATH="$HOME/opt/cross/bin:$PATH"

# 4. Build and run
cd MyOS
make clean
make all
make run
```

## 📚 Learning Resources

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development guide
- [Writing a Simple Operating System from Scratch](https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf) - Great tutorial
- [Bran's Kernel Development Tutorial](http://www.osdever.net/bkerndev/Docs/title.htm) - Step-by-step guide

## 🆘 Getting Help

If you encounter issues:

1. **Check the build logs** for specific error messages
2. **Start with minimal components** (bootloader only)
3. **Test tools individually** (compile simple C file with cross-compiler)
4. **Use QEMU monitor** for debugging (`Ctrl+Alt+2` in QEMU)

Your MyOS project is quite comprehensive! Start with the basic setup and gradually add components as you verify each part works.