#include "FreeRTOSConfig.h"

#define XT_STK_PC      0x00
#define XT_STK_PS      0x04
#define XT_STK_A0      0x08
#define XT_STK_A1      0x0C
#define XT_STK_A2      0x10
#define XT_STK_A3      0x14
#define XT_STK_A4      0x18
#define XT_STK_A5      0x1C
#define XT_STK_A6      0x20
#define XT_STK_A7      0x24
#define XT_STK_A8      0x28
#define XT_STK_A9      0x2C
#define XT_STK_A10     0x30
#define XT_STK_A11     0x34
#define XT_STK_A12     0x38
#define XT_STK_A13     0x3C
#define XT_STK_A14     0x40
#define XT_STK_A15     0x44
#define XT_STK_SAR     0x48
#define XT_STK_EXIT    0x4C
#define XT_STK_FRMSZ   0x50

#define XT_SOL_PC      0x00
#define XT_SOL_PS      0x04
#define XT_SOL_A12     0x08
#define XT_SOL_A13     0x0C
#define XT_SOL_A14     0x10
#define XT_SOL_A15     0x14
#define XT_SOL_EXIT    0x18
#define XT_SOL_FRMSZ   0x1C

#define TOPOFSTACK_OFFS 0x00

.section .vectors, "ax"
.align 256
.global _vector_table
_vector_table:
    j _start
    .space 29
    j _frxt_timer_int
    .space 221

.section .text, "ax"

.extern pxCurrentTCB
.extern vTaskSwitchContext
.extern xPortSysTickHandler
.extern port_switch_flag
.extern port_interrupt_sp
.extern port_interrupt_nesting

.global vPortYield
.type vPortYield, @function
.align 4
vPortYield:
    addi    sp, sp, -XT_SOL_FRMSZ
    rsr     a2, PS
    s32i    a0, sp, XT_SOL_PC
    s32i    a2, sp, XT_SOL_PS
    s32i    a12, sp, XT_SOL_A12
    s32i    a13, sp, XT_SOL_A13
    s32i    a14, sp, XT_SOL_A14
    s32i    a15, sp, XT_SOL_A15
    movi    a2, 0
    s32i    a2, sp, XT_SOL_EXIT
    movi    a2, pxCurrentTCB
    l32i    a2, a2, 0
    s32i    sp, a2, TOPOFSTACK_OFFS
    call0   _frxt_dispatch

.global _frxt_dispatch
.type _frxt_dispatch, @function
.align 4
_frxt_dispatch:
    call0   vTaskSwitchContext
    movi    a2, pxCurrentTCB
    l32i    a2, a2, 0
    l32i    sp, a2, TOPOFSTACK_OFFS
    l32i    a2, sp, XT_STK_EXIT
    bnez    a2, .Linterrupt_frame
.Lsolicited_frame:
    l32i    a12, sp, XT_SOL_A12
    l32i    a13, sp, XT_SOL_A13
    l32i    a14, sp, XT_SOL_A14
    l32i    a15, sp, XT_SOL_A15
    l32i    a2,  sp, XT_SOL_PS
    wsr     a2,  PS
    rsync
    l32i    a0,  sp, XT_SOL_PC
    addi    sp,  sp, XT_SOL_FRMSZ
    ret
.Linterrupt_frame:
    mov     a2,  sp
    l32i    a3,  a2, XT_STK_PC
    wsr     a3,  EPC1
    l32i    a3,  a2, XT_STK_SAR
    wsr     a3,  SAR
    l32i    a0,  a2, XT_STK_A0
    l32i    a1,  a2, XT_STK_A1
    l32i    a3,  a2, XT_STK_A3
    l32i    a4,  a2, XT_STK_A4
    l32i    a5,  a2, XT_STK_A5
    l32i    a6,  a2, XT_STK_A6
    l32i    a7,  a2, XT_STK_A7
    l32i    a8,  a2, XT_STK_A8
    l32i    a9,  a2, XT_STK_A9
    l32i    a10, a2, XT_STK_A10
    l32i    a11, a2, XT_STK_A11
    l32i    a12, a2, XT_STK_A12
    l32i    a13, a2, XT_STK_A13
    l32i    a14, a2, XT_STK_A14
    l32i    a15, a2, XT_STK_A15
    l32i    a2,  a2, XT_STK_A2
    rfe

.global _frxt_timer_int
.type _frxt_timer_int, @function
.align 4
_frxt_timer_int:
    wsr     a0, EXCSAVE1
    addi    sp, sp, -XT_STK_FRMSZ
    rsr     a0, EPC1
    s32i    a0, sp, XT_STK_PC
    rsr     a0, PS
    s32i    a0, sp, XT_STK_PS
    rsr     a0, EXCSAVE1
    s32i    a0, sp, XT_STK_A0
    addi    a0, sp, XT_STK_FRMSZ
    s32i    a0, sp, XT_STK_A1
    s32i    a2, sp, XT_STK_A2
    s32i    a3, sp, XT_STK_A3
    s32i    a4, sp, XT_STK_A4
    s32i    a5, sp, XT_STK_A5
    s32i    a6, sp, XT_STK_A6
    s32i    a7, sp, XT_STK_A7
    s32i    a8, sp, XT_STK_A8
    s32i    a9, sp, XT_STK_A9
    s32i    a10, sp, XT_STK_A10
    s32i    a11, sp, XT_STK_A11
    s32i    a12, sp, XT_STK_A12
    s32i    a13, sp, XT_STK_A13
    s32i    a14, sp, XT_STK_A14
    s32i    a15, sp, XT_STK_A15
    rsr     a0, SAR
    s32i    a0, sp, XT_STK_SAR
    movi    a0, 1
    s32i    a0, sp, XT_STK_EXIT
    movi    a0, pxCurrentTCB
    l32i    a0, a0, 0
    s32i    sp, a0, TOPOFSTACK_OFFS
    movi    a0, port_interrupt_nesting
    l32i    a2, a0, 0
    addi    a2, a2, 1
    s32i    a2, a0, 0
    movi    a0, port_interrupt_sp
    l32i    sp, a0, 0
    rsr     a0, PS
    movi    a2, ~1
    and     a0, a0, a2
    wsr     a0, PS
    rsync
    call0   xPortSysTickHandler
    rsil    a0, 15
    movi    a0, port_interrupt_nesting
    l32i    a2, a0, 0
    addi    a2, a2, -1
    s32i    a2, a0, 0
    movi    a0, port_switch_flag
    l32i    a2, a0, 0
    beqz    a2, .Lrestore_task
    movi    a3, 0
    s32i    a3, a0, 0
    call0   vTaskSwitchContext
.Lrestore_task:
    movi    a0, pxCurrentTCB
    l32i    a0, a0, 0
    l32i    sp, a0, TOPOFSTACK_OFFS
    mov     a2, sp
    l32i    a3,  a2, XT_STK_PC
    wsr     a3,  EPC1
    l32i    a3,  a2, XT_STK_SAR
    wsr     a3,  SAR
    l32i    a0,  a2, XT_STK_A0
    l32i    a1,  a2, XT_STK_A1
    l32i    a3,  a2, XT_STK_A3
    l32i    a4,  a2, XT_STK_A4
    l32i    a5,  a2, XT_STK_A5
    l32i    a6,  a2, XT_STK_A6
    l32i    a7,  a2, XT_STK_A7
    l32i    a8,  a2, XT_STK_A8
    l32i    a9,  a2, XT_STK_A9
    l32i    a10, a2, XT_STK_A10
    l32i    a11, a2, XT_STK_A11
    l32i    a12, a2, XT_STK_A12
    l32i    a13, a2, XT_STK_A13
    l32i    a14, a2, XT_STK_A14
    l32i    a15, a2, XT_STK_A15
    l32i    a2,  a2, XT_STK_A2
    rfe

.global vPortYieldFromInt
.type vPortYieldFromInt, @function
.align 4
vPortYieldFromInt:
    movi    a2, port_switch_flag
    movi    a3, 1
    s32i    a3, a2, 0
    ret

.global vPortAssertFailed
.type vPortAssertFailed, @function
.align 4
vPortAssertFailed:
    waiti   0
    j       vPortAssertFailed

.section .bss
.align 16
.global port_switch_flag
port_switch_flag:
    .space  4

.global port_interrupt_sp
port_interrupt_sp:
    .space  4

.global port_interrupt_nesting
port_interrupt_nesting:
    .space  4

.global port_isr_stack
port_isr_stack:
    .space  2048
