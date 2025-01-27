#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define a boolean type as uint8_t (since C doesn't have a built-in bool type).
typedef uint8_t bool;
#define true 1
#define false 0

// Define a structure to represent the boot sector of a FAT12 file system.
typedef struct 
{
    uint8_t BootJumpInstruction[3]; // Jump instruction to boot code.
    uint8_t OemIdentifier[8];       // OEM identifier (e.g., "MSDOS5.0").
    uint16_t BytesPerSector;        // Bytes per sector (usually 512).
    uint8_t SectorsPerCluster;      // Sectors per cluster.
    uint16_t ReservedSectors;       // Number of reserved sectors (usually 1).
    uint8_t FatCount;               // Number of File Allocation Tables (usually 2).
    uint16_t DirEntryCount;         // Number of root directory entries.
    uint16_t TotalSectors;          // Total number of sectors (0 for FAT32).
    uint8_t MediaDescriptorType;    // Media descriptor type.
    uint16_t SectorsPerFat;         // Sectors per FAT table.
    uint16_t SectorsPerTrack;       // Number of sectors per track (used for CHS).
    uint16_t Heads;                 // Number of heads (used for CHS).
    uint32_t HiddenSectors;         // Number of hidden sectors.
    uint32_t LargeSectorCount;      // Total sectors for large disks (FAT32).

    // Extended boot record fields.
    uint8_t DriveNumber;            // BIOS drive number.
    uint8_t _Reserved;              // Reserved (not used).
    uint8_t Signature;              // Extended boot signature (0x29).
    uint32_t VolumeId;              // Volume serial number.
    uint8_t VolumeLabel[11];        // Volume label (11 characters, padded with spaces).
    uint8_t SystemId[8];            // File system type (e.g., "FAT12").

    // Code and unused fields are ignored in this structure.
} __attribute__((packed)) BootSector;

// Define a structure to represent a directory entry in the FAT12 file system.
typedef struct 
{
    uint8_t Name[11];              // File name (8.3 format, padded with spaces).
    uint8_t Attributes;            // File attributes (e.g., read-only, hidden).
    uint8_t _Reserved;             // Reserved (not used).
    uint8_t CreatedTimeTenths;     // Tenths of a second file was created.
    uint16_t CreatedTime;          // Time file was created.
    uint16_t CreatedDate;          // Date file was created.
    uint16_t AccessedDate;         // Date file was last accessed.
    uint16_t FirstClusterHigh;     // High 16 bits of first cluster (FAT32 only).
    uint16_t ModifiedTime;         // Time file was last modified.
    uint16_t ModifiedDate;         // Date file was last modified.
    uint16_t FirstClusterLow;      // Low 16 bits of first cluster.
    uint32_t Size;                 // File size in bytes.
} __attribute__((packed)) DirectoryEntry;

// Global variables to store the boot sector, FAT, and root directory.
BootSector g_BootSector;
uint8_t* g_Fat = NULL;
DirectoryEntry* g_RootDirectory = NULL;
uint32_t g_RootDirectoryEnd; // LBA address of the end of the root directory.

// Reads the boot sector from the disk image.
bool readBootSector(FILE* disk)
{
    return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
}

// Reads a specified number of sectors from the disk into a buffer.
bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) == count);
    return ok;
}

// Reads the FAT table into memory.
bool readFat(FILE* disk)
{
    g_Fat = (uint8_t*) malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
    return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_Fat);
}

// Reads the root directory into memory.
bool readRootDirectory(FILE* disk)
{
    uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;
    uint32_t sectors = (size / g_BootSector.BytesPerSector);
    if (size % g_BootSector.BytesPerSector > 0)
        sectors++;

    g_RootDirectoryEnd = lba + sectors;
    g_RootDirectory = (DirectoryEntry*) malloc(sectors * g_BootSector.BytesPerSector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}

// Finds a file in the root directory by its name.
DirectoryEntry* findFile(const char* name)
{
    for (uint32_t i = 0; i < g_BootSector.DirEntryCount; i++)
    {
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0)
            return &g_RootDirectory[i];
    }

    return NULL;
}

// Reads the content of a file into the output buffer.
bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer)
{
    bool ok = true;
    uint16_t currentCluster = fileEntry->FirstClusterLow;

    do {
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        else
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;

    } while (ok && currentCluster < 0x0FF8);

    return ok;
}

// Main function: Reads a file from a FAT12 disk image.
int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
        return -1;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        return -3;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read root directory!\n");
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Could not read file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        free(buffer);
        return -5;
    }

    for (size_t i = 0; i < fileEntry->Size; i++)
    {
        if (isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("<%02x>", buffer[i]);
    }
    printf("\n");

    free(buffer);
    free(g_Fat);
    free(g_RootDirectory);
    return 0;
}
