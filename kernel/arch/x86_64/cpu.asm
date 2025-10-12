; CPU utility functions

section .text
bits 32

global cpu_halt

cpu_halt:
    hlt
    ret
