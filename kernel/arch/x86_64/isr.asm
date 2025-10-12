; Interrupt Service Routines

section .text
bits 32

; External C functions
extern isr_handler
extern irq_handler

; Macro for ISRs without error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push 0              ; Push dummy error code
    push %1             ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro for ISRs with error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push %1             ; Push interrupt number
    jmp isr_common_stub
%endmacro

; CPU Exceptions (0-31)
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Into Detected Overflow
ISR_NOERRCODE 5     ; Out of Bounds
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; No Coprocessor
ISR_ERRCODE   8     ; Double Fault
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Bad TSS
ISR_ERRCODE   11    ; Segment Not Present
ISR_ERRCODE   12    ; Stack Fault
ISR_ERRCODE   13    ; General Protection Fault
ISR_ERRCODE   14    ; Page Fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; Coprocessor Fault
ISR_ERRCODE   17    ; Alignment Check
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; Reserved
ISR_NOERRCODE 20    ; Reserved
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE   30    ; Security Exception
ISR_NOERRCODE 31    ; Reserved

; Macro for IRQs
%macro IRQ 2
global irq%1
irq%1:
    cli
    push 0              ; Push dummy error code
    push %2             ; Push IRQ number
    jmp irq_common_stub
%endmacro

; Hardware interrupts (IRQ 0-15)
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Common ISR stub
isr_common_stub:
    pusha               ; Push all general purpose registers
    
    mov ax, ds
    push eax            ; Save data segment
    
    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call isr_handler    ; Call C handler
    
    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                ; Pop all general purpose registers
    add esp, 8          ; Clean up error code and interrupt number
    sti
    iret                ; Return from interrupt

; Common IRQ stub
irq_common_stub:
    pusha               ; Push all general purpose registers
    
    mov ax, ds
    push eax            ; Save data segment
    
    mov ax, 0x10        ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call irq_handler    ; Call C handler
    
    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                ; Pop all general purpose registers
    add esp, 8          ; Clean up error code and interrupt number
    sti
    iret                ; Return from interrupt
