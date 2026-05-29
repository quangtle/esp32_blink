.section .text._start, "ax"
.global _start
.type _start, @function
.align 4

_start:
    movi    a0, 0
    wsr.intenable a0
    wsr.ps        a0

    movi    a2, 0x3FF480A8
    movi    a3, 0x3FF48090
    movi    a0, 0x50D83AA1
    movi    a4, 0
    s32i.n  a0, a2, 0
    s32i.n  a4, a3, 0
    s32i.n  a4, a2, 0

    movi    a2, 0x3FF5F064
    movi    a3, 0x3FF5F048
    s32i.n  a0, a2, 0
    s32i.n  a4, a3, 0

    movi    a2, 0x3FF60064
    movi    a3, 0x3FF60048
    s32i.n  a0, a2, 0
    s32i.n  a4, a3, 0

    movi    sp, _stack_start
    call0   boot_main

halt:
    waiti   0
    j       halt
