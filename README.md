# OS Development Bootloader

A simple x86 operating system bootloader and kernel written in assembly, along with a FAT12 disk image utility in C. Start to learn a new thing and how computers work thanks to NanoByte (YouTube channel). Check out the NanoByte playlist on [YouTube](https://www.youtube.com/watch?v=9t-SPC7Tczc&list=PLFjM7v6KGMpjWYxnihhd4P3G0BhMB9ReG).

## Project Structure
```
OsDev/
├── build/ # Build output directory (e.g., main_floppy.img)
├── src/
│   ├── bootloader/
│   │   └── boot.asm # Bootloader (FAT12 compliant, loads kernel)
│   └── kernel/
│       └── main.asm # Kernel (prints "Hello world from KERNEL!")
├── tools/
│   └── fat/
│       └── fat.c # FAT12 disk image reader utility
├── run.sh # QEMU script to boot the OS
├── debug.sh # Bochs debugger script
├── bx_enh_dbg.ini # Bochs debugger configuration
└── .vscode/tasks.json # VSCode task configuration (example)
```

## Key Components

### 1. Bootloader (`src/bootloader/boot.asm`)
- **FAT12 compliant**: Reads kernel from disk using BIOS interrupts.
- **Loads kernel**: Searches for `KERNEL.BIN` in the root directory, loads it into memory at `0x2000:0x0000`, and transfers control.
- **Error handling**: Displays messages for disk read failures or missing kernel.

### 2. Kernel (`src/kernel/main.asm`)
- **16-bit Real Mode**: Simple kernel that prints "Hello world from KERNEL!" using BIOS interrupt `0x10`.
- **Basic I/O**: Implements a `puts` function for string output.

### 3. FAT12 Utility (`tools/fat/fat.c`)
- **Disk image parser**: Reads FAT12 disk images and extracts files.
- **Usage**: 
  ```bash
  ./fat <disk_image> <file_name>  # Extracts and prints file contents
  ```

## Build & Run

### Dependencies
- **NASM** (for assembly)
- **QEMU** (for emulation)
- **Bochs** (for debugging)
- **GCC** (to compile `fat.c`)

### Steps

1. To build the project and create the disk image, simply run:
   ```bash
   make
   ```

2. If you need to clean the build directory and start fresh, use:
   ```bash
   make clean
   make
   ```

3. Run the OS:
   ```bash
   ./run.sh    # Uses QEMU
   OR
   ./debug.sh  # Uses Bochs for debugging
   ```

## Debugging

- **Bochs Configuration**: Configured in `bx_enh_dbg.ini` to show registers, I/O windows, and disassembly.
- **QEMU**: Quick emulation with `run.sh`.

## Notes
- The kernel runs in 16-bit Real Mode and uses BIOS interrupts.
- The FAT12 utility helps manage the disk image during development.
- The bootloader assumes the kernel is named `KERNEL.BIN` and is placed in the root directory of the disk image.

Clone the repo, follow the build steps, and explore the code to see how a minimal OS boots! 🚀

## Tools
- **Bochs**: [Bochs Emulator](http://bochs.sourceforge.net/)
- **mtools**: [GNU mtools](https://www.gnu.org/software/mtools/)
- **VSCode**: [Visual Studio Code](https://code.visualstudio.com/)
- **Okteta (Linux)**: [Okteta Hex Editor](https://apps.kde.org/en/okteta)
- **HxD (Windows)**: [HxD Hex Editor](https://mh-nexus.de/en/hxd/)

