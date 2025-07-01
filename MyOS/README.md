# MyOS - Simple Operating System

MyOS adalah sistem operasi sederhana yang dibangun dari nol (from scratch) untuk tujuan pembelajaran dan eksperimen. Sistem operasi ini memiliki kernel yang mendukung multitasking, manajemen memori, sistem file, dan antarmuka grafis (GUI).

## 🚀 Fitur Utama

### Kernel
- **Bootloader kustom** - Bootloader assembly yang memuat kernel
- **Manajemen memori** - Physical dan virtual memory management dengan paging
- **Multitasking** - Process scheduler dengan preemptive multitasking
- **Sistem file** - Virtual File System (VFS) dengan dukungan berbagai format
- **Driver perangkat** - Driver untuk keyboard, mouse, VGA, dan disk
- **Interrupt handling** - Penanganan interrupt dan system calls

### GUI Desktop Environment
- **Window manager** - Manajemen window dengan drag, resize, minimize
- **Desktop environment** - Desktop dengan taskbar dan wallpaper
- **Aplikasi built-in**:
  - File Manager - Menjelajahi dan mengelola file
  - Text Editor - Editor teks sederhana
  - Calculator - Kalkulator dengan GUI
  - Terminal - Emulator terminal dalam GUI

### Command Line Interface
- **Shell interaktif** - Command line shell dengan berbagai perintah
- **Built-in commands**:
  - `ls`, `cd`, `pwd` - Navigasi direktori
  - `cat`, `echo` - Manipulasi file dan teks
  - `mkdir`, `rm`, `cp`, `mv` - Operasi file dan direktori
  - `ps`, `kill` - Manajemen proses
  - `free`, `uname`, `date` - Informasi sistem

### Networking (Dalam Pengembangan)
- **TCP/IP Stack** - Implementasi protokol jaringan
- **Socket API** - Interface pemrograman jaringan
- **Ethernet driver** - Driver untuk kartu jaringan

## 📋 Prasyarat

### Perangkat Lunak yang Diperlukan

1. **Cross-compiler untuk i686-elf**
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential
   ```

2. **NASM Assembler**
   ```bash
   sudo apt install nasm
   ```

3. **QEMU Emulator**
   ```bash
   sudo apt install qemu-system-x86
   ```

4. **GRUB untuk membuat ISO**
   ```bash
   sudo apt install grub-pc-bin xorriso mtools
   ```

### Spesifikasi Minimum
- RAM: 128 MB (untuk QEMU)
- Storage: 50 MB untuk source code dan build
- OS Host: Linux (Ubuntu/Debian recommended)

## 🛠️ Instalasi dan Build

### 1. Clone Repository
```bash
git clone https://github.com/username/myos.git
cd myos
```

### 2. Install Dependencies
```bash
make install-deps
```

### 3. Build Cross-Compiler (Jika belum ada)
Script akan mengunduh dan membangun cross-compiler i686-elf:
```bash
# Script install-deps akan menangani ini
export PATH="$HOME/opt/cross/bin:$PATH"
```

### 4. Build Operating System
```bash
# Build semua komponen
make all

# Atau tahap demi tahap
make linker.ld        # Buat linker script
make boot-config      # Buat konfigurasi boot
make                  # Build lengkap
```

### 5. Jalankan di QEMU
```bash
# Jalankan dari disk image
make run

# Atau dari ISO
make run-iso

# Debug mode
make debug
```

## 📁 Struktur Proyek

```
MyOS/
├── boot/                 # Bootloader
│   └── bootloader.asm   # Assembly bootloader
├── kernel/              # Kernel core
│   ├── core/           # Kernel utama
│   ├── memory/         # Manajemen memori
│   ├── process/        # Manajemen proses
│   ├── drivers/        # Device drivers
│   ├── filesystem/     # Sistem file
│   └── include/        # Header files
├── userspace/           # User space programs
│   ├── lib/            # Libraries (libc, libgui)
│   ├── shell/          # Command line shell
│   ├── gui/            # GUI applications
│   └── utilities/      # System utilities
├── tools/               # Build tools dan scripts
├── config/              # File konfigurasi
└── docs/               # Dokumentasi
```

## 🎮 Cara Menggunakan

### Boot Process
1. System akan boot dengan bootloader kustom
2. Kernel dimuat dan dijalankan
3. Sistem inisialisasi (memory, drivers, filesystem)
4. GUI desktop environment dimulai

### Desktop Environment
- **Taskbar**: Klik tombol untuk membuka aplikasi
- **Window Management**: Drag untuk memindah, klik X untuk menutup
- **Applications**: File manager, text editor, calculator, terminal

### Command Line
Buka terminal dari taskbar atau tekan Ctrl+Alt+T:
```bash
myos$ help                    # Lihat semua perintah
myos$ ls                      # List files
myos$ cd /home                # Change directory
myos$ cat README.txt          # Baca file
myos$ ps                      # Lihat proses
myos$ free                    # Cek memory usage
```

## 🔧 Development

### Menambah Fitur Baru

1. **Kernel Module Baru**:
   ```c
   // kernel/new_module/module.c
   #include "../include/kernel.h"
   
   void init_new_module(void) {
       // Implementation
   }
   ```

2. **GUI Application Baru**:
   ```c
   // userspace/gui/applications/new_app/
   #include "../../lib/libgui/window.h"
   
   int main(void) {
       Window* win = create_window("New App", 100, 100, 400, 300);
       // Application logic
   }
   ```

3. **Shell Command Baru**:
   Tambahkan ke `userspace/shell/shell.c`:
   ```c
   int cmd_new_command(int argc, char** argv) {
       // Command implementation
       return 1;
   }
   ```

### Testing

```bash
# Test bootloader saja
make test-boot

# Test kernel saja
make test-kernel

# Clean dan rebuild
make clean && make all

# Generate dokumentasi
make docs
```

## 📊 Performance dan Limitations

### Current Limitations
- **Memory**: Maximum 128MB RAM
- **Storage**: Simple VFS, belum ada persistent storage
- **Network**: Dalam tahap pengembangan
- **Graphics**: Basic VGA mode, belum ada hardware acceleration
- **Sound**: Belum diimplementasikan

### Performance Tips
- Gunakan QEMU dengan KVM untuk performa lebih baik
- Alokasi memory yang efisien dalam aplikasi
- Hindari operasi I/O berlebihan dalam loop

## 🐛 Troubleshooting

### Build Errors
```bash
# Error: i686-elf-gcc not found
export PATH="$HOME/opt/cross/bin:$PATH"

# Error: Permission denied
chmod +x tools/*.sh

# Error: Out of memory
# Tingkatkan memory QEMU: -m 256M
```

### Runtime Issues
```bash
# System hang atau crash
make debug  # Jalankan dengan debugger

# GUI tidak muncul
# Periksa graphics initialization di kernel

# Keyboard tidak responsif
# Periksa interrupt handling
```

## 🤝 Contributing

1. Fork repository ini
2. Buat branch untuk fitur baru (`git checkout -b feature/amazing-feature`)
3. Commit perubahan (`git commit -m 'Add amazing feature'`)
4. Push ke branch (`git push origin feature/amazing-feature`)
5. Buat Pull Request

### Coding Standards
- Gunakan 4 spasi untuk indentasi
- Nama function dan variable dalam snake_case
- Nama constant dalam UPPER_CASE
- Dokumentasi untuk semua public functions

## 📜 License

Project ini dilisensikan under MIT License - lihat file [LICENSE](LICENSE) untuk detail.

## 🙏 Acknowledgments

- **OSDev Community** - Dokumentasi dan tutorial
- **James Molloy's Kernel Development Tutorial**
- **Intel x86 Architecture Manual**
- **Linux Kernel** - Referensi implementasi
- **QEMU Project** - Emulator untuk testing

## 📚 Resources untuk Belajar

### Books
- "Operating Systems: Design and Implementation" - Andrew Tanenbaum
- "Understanding the Linux Kernel" - Daniel P. Bovet
- "Intel 64 and IA-32 Architectures Software Developer Manual"

### Online Resources
- [OSDev Wiki](https://wiki.osdev.org/)
- [Bran's Kernel Development Tutorial](http://www.osdever.net/bkerndev/)
- [Intel Developer Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)

### Communities
- [OSDev Forum](https://forum.osdev.org/)
- [Reddit r/osdev](https://www.reddit.com/r/osdev/)
- [Stack Overflow - Operating Systems](https://stackoverflow.com/questions/tagged/operating-system)

## 🗺️ Roadmap

### Version 1.1 (Next Release)
- [ ] Persistent file system (EXT2/FAT32)
- [ ] Better memory management
- [ ] More GUI applications
- [ ] Network stack completion
- [ ] Sound system

### Version 1.2 (Future)
- [ ] USB support
- [ ] Multi-core support
- [ ] 64-bit architecture
- [ ] Hardware accelerated graphics
- [ ] Package manager

### Version 2.0 (Long Term)
- [ ] POSIX compliance
- [ ] X11 compatibility layer
- [ ] Virtual machine support
- [ ] Microkernel architecture

---

**MyOS v1.0** - Built with ❤️ for learning and exploration

For questions or support, please create an issue in the GitHub repository.