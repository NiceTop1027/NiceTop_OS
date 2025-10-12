; NiceTop_OS Bootloader
; Multiboot2 header for GRUB

section .multiboot
align 8
multiboot_start:
    dd 0xe85250d6                ; magic number
    dd 0                         ; architecture (0 = i386)
    dd multiboot_end - multiboot_start  ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (multiboot_end - multiboot_start))  ; checksum

    ; Framebuffer tag
    align 8
framebuffer_tag_start:
    dw 5    ; type = framebuffer
    dw 0    ; flags
    dd framebuffer_tag_end - framebuffer_tag_start  ; size
    dd 1024 ; width
    dd 768  ; height
    dd 32   ; depth (bits per pixel)
framebuffer_tag_end:

    ; End tag
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_end:

section .bss
align 16
stack_bottom:
    resb 16384  ; 16 KB stack
stack_top:

section .text
bits 32
global _start
extern kernel_main

_start:
    ; Set up stack (32-bit)
    mov esp, stack_top
    mov ebp, esp

    ; Clear direction flag
    cld

    ; Save multiboot info (32-bit registers)
    push ebx  ; multiboot info pointer
    push eax  ; multiboot magic

    ; Call kernel main
    call kernel_main

    ; Hang if kernel returns
.hang:
    cli
    hlt
    jmp .hang
