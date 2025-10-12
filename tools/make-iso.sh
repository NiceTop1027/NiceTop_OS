#!/bin/bash
# Create bootable ISO with both BIOS and UEFI support

ISO_DIR="$1"
OUTPUT_ISO="$2"

# Use i686-elf-grub-mkrescue which supports i386-pc (BIOS)
i686-elf-grub-mkrescue -o "$OUTPUT_ISO" "$ISO_DIR" \
    --modules="multiboot2 biosdisk iso9660" \
    2>&1 | grep -v "xorriso" || true
