; Multiboot 헤더 상수
MBALIGN     equ 1 << 0
MEMINFO     equ 1 << 1
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16KB 스택
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    
    ; 멀티부트 정보 저장
    push ebx
    push eax
    
    ; 커널 메인 호출
    call kernel_main
    
    ; 시스템 중지
    cli
.hang:
    hlt
    jmp .hang