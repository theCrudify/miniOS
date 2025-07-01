const fs = require('fs');
const path = require('path');

const structure = {
  "MyOS": {
    "boot": {
      "bootloader.asm": "",
      "boot.cfg": "",
      "grub.cfg": ""
    },
    "kernel": {
      "include": {
        "kernel.h": "",
        "memory.h": "",
        "process.h": "",
        "filesystem.h": "",
        "network.h": "",
        "graphics.h": ""
      },
      "core": {
        "kernel.c": "",
        "interrupt.c": "",
        "syscall.c": "",
        "timer.c": ""
      },
      "memory": {
        "paging.c": "",
        "heap.c": "",
        "physical.c": ""
      },
      "process": {
        "scheduler.c": "",
        "thread.c": "",
        "ipc.c": ""
      },
      "drivers": {
        "keyboard.c": "",
        "mouse.c": "",
        "vga.c": "",
        "disk.c": "",
        "network.c": ""
      },
      "filesystem": {
        "vfs.c": "",
        "ext2.c": "",
        "fat32.c": ""
      }
    },
    "userspace": {
      "lib": {
        "libc": {
          "stdio.c": "",
          "stdlib.c": "",
          "string.c": "",
          "math.c": ""
        },
        "libgui": {
          "window.c": "",
          "button.c": "",
          "menu.c": "",
          "graphics.c": ""
        }
      },
      "shell": {
        "shell.c": "",
        "commands.c": "",
        "parser.c": ""
      },
      "gui": {
        "desktop": {
          "desktop.c": "",
          "taskbar.c": "",
          "wallpaper.c": ""
        },
        "window_manager": {
          "wm.c": "",
          "compositor.c": "",
          "effects.c": ""
        },
        "applications": {
          "file_manager": {},
          "text_editor": {},
          "calculator": {},
          "terminal": {}
        }
      },
      "utilities": {
        "ps.c": "",
        "ls.c": "",
        "cp.c": "",
        "mv.c": "",
        "rm.c": ""
      }
    },
    "tools": {
      "build.sh": "",
      "cross-compiler": {},
      "disk-image.sh": "",
      "qemu-test.sh": ""
    },
    "config": {
      "system.conf": "",
      "network.conf": "",
      "users.conf": ""
    },
    "docs": {
      "architecture.md": "",
      "api.md": "",
      "building.md": ""
    },
    "tests": {
      "unit": {},
      "integration": {},
      "performance": {}
    },
    "Makefile": "",
    "README.md": "",
    "LICENSE": ""
  }
};

function createStructure(basePath, obj) {
  for (const name in obj) {
    const fullPath = path.join(basePath, name);
    if (typeof obj[name] === "string") {
      fs.writeFileSync(fullPath, obj[name]);
      console.log(`Created file: ${fullPath}`);
    } else {
      if (!fs.existsSync(fullPath)) fs.mkdirSync(fullPath);
      console.log(`Created folder: ${fullPath}`);
      createStructure(fullPath, obj[name]);
    }
  }
}

createStructure('.', structure);
