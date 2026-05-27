.section .text._start, "ax"
.global _start
.type _start, @function
.align 4

_start:
    /* ---- RTC watchdog (0x3FF48000 block) ---- */
    movi    a2, 0x3FF480A8
    movi    a3, 0x3FF4808C
    movi    a0, 0x50D83AA1
    movi    a4, 0
    s32i.n  a0, a2, 0
    s32i.n  a4, a3, 0

    /* ---- Timer Group 0 watchdog (0x3FF5F000 block) ---- */
    movi    a2, 0x3FF5F064
    movi    a3, 0x3FF5F048
    s32i.n  a0, a2, 0
    s32i.n  a4, a3, 0

    /* ---- Timer Group 1 watchdog (0x3FF60000 block) ---- */
    movi    a2, 0x3FF60064
    movi    a3, 0x3FF60048
    s32i.n  a0, a2, 0
    s32i.n  a4, a3, 0

    /* ---- CPU configuration ---- */
    movi    a0, 0
    wsr.intenable a0
    wsr.ps        a0

    /* ---- Stack pointer ---- */
    movi    sp, _stack_start

    /* ---- Copy .data from flash/IRAM to DRAM (if any) ---- */
    movi    a2, _data_start
    movi    a3, _data_end
    movi    a4, _data_load
    bgeu    a2, a3, 2f
1:
    l32i    a5, a4, 0
    s32i    a5, a2, 0
    addi    a2, a2, 4
    addi    a4, a4, 4
    bltu    a2, a3, 1b
2:

    /* ---- Zero .bss section ---- */
    movi    a2, _bss_start
    movi    a3, _bss_end
    bgeu    a2, a3, 2f
1:
    s32i    a0, a2, 0
    addi    a2, a2, 4
    bltu    a2, a3, 1b
2:

    /* ---- Enter C land ---- */
    call0   main

halt:
    waiti   0
    j       halt
