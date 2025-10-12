; GDT flush function
; Loads the GDT and reloads segment registers

section .text
bits 32

global gdt_flush

gdt_flush:
    mov eax, [esp + 4]  ; Get the pointer to the GDT, passed as a parameter
    lgdt [eax]          ; Load the GDT pointer

    ; Reload segment registers
    mov ax, 0x10        ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Far jump to reload CS (code segment)
    jmp 0x08:.flush     ; 0x08 is the offset to our code segment

.flush:
    ret
