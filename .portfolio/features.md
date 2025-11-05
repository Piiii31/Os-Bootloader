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
