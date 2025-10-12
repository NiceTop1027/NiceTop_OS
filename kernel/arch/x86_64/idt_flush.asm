; IDT flush function

section .text
bits 32

global idt_flush

idt_flush:
    mov eax, [esp + 4]  ; Get the pointer to the IDT
    lidt [eax]          ; Load the IDT
    ret
