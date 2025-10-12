# NiceTop_OS Makefile

# Compiler and tools (cross-compiler for x86_64)
CC = x86_64-elf-gcc
CXX = x86_64-elf-g++
AS = nasm
LD = x86_64-elf-gcc
GRUB_MKRESCUE = grub-mkrescue

# Flags
CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
         -Wall -Wextra -Ikernel/include -Iinclude
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = -f elf32
LDFLAGS = -m32 -ffreestanding -nostdlib -T kernel/linker.ld

# Directories
BUILD_DIR = build
KERNEL_DIR = kernel
BOOT_DIR = boot
ISO_DIR = $(BUILD_DIR)/iso

# Source files
ASM_SOURCES = $(wildcard $(BOOT_DIR)/*.asm) $(wildcard $(KERNEL_DIR)/arch/x86_64/*.asm)
C_SOURCES = $(wildcard $(KERNEL_DIR)/*.c) $(wildcard $(KERNEL_DIR)/*/*.c)
CXX_SOURCES = $(wildcard $(KERNEL_DIR)/*.cpp) $(wildcard $(KERNEL_DIR)/*/*.cpp)

# Object files
ASM_OBJECTS = $(patsubst %.asm, $(BUILD_DIR)/%.o, $(ASM_SOURCES))
C_OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
CXX_OBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CXX_SOURCES))

ALL_OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS) $(CXX_OBJECTS)

# Output
KERNEL_ELF = $(BUILD_DIR)/nicetop.elf
KERNEL_BIN = $(BUILD_DIR)/nicetop.bin
ISO_FILE = $(BUILD_DIR)/nicetop.iso

.PHONY: all clean run iso dirs

all: dirs $(KERNEL_BIN)

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/boot
	@mkdir -p $(BUILD_DIR)/kernel
	@mkdir -p $(BUILD_DIR)/kernel/arch/x86_64
	@mkdir -p $(BUILD_DIR)/kernel/drivers
	@mkdir -p $(BUILD_DIR)/kernel/memory
	@mkdir -p $(BUILD_DIR)/kernel/graphics
	@mkdir -p $(BUILD_DIR)/kernel/gui
	@mkdir -p $(ISO_DIR)/boot/grub

# Compile assembly
$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Compile C
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link kernel to ELF
$(KERNEL_ELF): $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Convert ELF to binary (for now, just use ELF directly with multiboot2)
$(KERNEL_BIN): $(KERNEL_ELF)
	@cp $(KERNEL_ELF) $(KERNEL_BIN)

# Create ISO
iso: $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	@echo 'set timeout=3' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "NiceTop OS" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot2 /boot/nicetop.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	./tools/make-iso.sh $(ISO_DIR) $(ISO_FILE)

# Run in QEMU
run: iso
	qemu-system-x86_64 -cdrom $(ISO_FILE) -m 512M

# Debug in QEMU
debug: iso
	qemu-system-x86_64 -cdrom $(ISO_FILE) -m 512M -s -S

clean:
	rm -rf $(BUILD_DIR)

# Print variables for debugging
print-%:
	@echo $* = $($*)
