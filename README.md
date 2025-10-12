# NiceTop_OS

A modern GUI operating system built from scratch, inspired by Windows architecture.

## Architecture
- **Language**: C/C++, Assembly
- **Target**: x86_64 (64-bit)
- **Bootloader**: GRUB2 (Multiboot2)
- **GUI**: Custom graphics stack

## Project Structure
```
NiceTop_OS/
├── boot/           # Bootloader code
├── kernel/         # Kernel core
│   ├── arch/       # Architecture-specific code (x86_64)
│   ├── drivers/    # Device drivers
│   ├── memory/     # Memory management
│   ├── graphics/   # Graphics subsystem
│   └── gui/        # GUI framework
├── include/        # Header files
├── lib/            # Standard library
└── build/          # Build output
```

## Build Requirements
- GCC cross-compiler (x86_64-elf)
- NASM (assembler)
- GRUB2
- QEMU (for testing)
- Make

## Build & Run
```bash
make all
make run
```

## Development Roadmap
- [x] Project setup
- [ ] Bootloader
- [ ] Kernel initialization
- [ ] VGA text output
- [ ] Interrupt handling
- [ ] Memory management
- [ ] Graphics mode
- [ ] GUI framework
- [ ] Window manager
- [ ] Desktop environment

## License
MIT License
