#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

#define TIMG1_BASE              0x3FF60000UL
#define TIMG1_T0CONFIG          (*(volatile uint32_t *)(TIMG1_BASE + 0x00))
#define TIMG1_T0LO              (*(volatile uint32_t *)(TIMG1_BASE + 0x04))
#define TIMG1_T0HI              (*(volatile uint32_t *)(TIMG1_BASE + 0x08))
#define TIMG1_T0ALARMLO         (*(volatile uint32_t *)(TIMG1_BASE + 0x10))
#define TIMG1_T0ALARMHI         (*(volatile uint32_t *)(TIMG1_BASE + 0x14))
#define TIMG1_T0LOADLO          (*(volatile uint32_t *)(TIMG1_BASE + 0x18))
#define TIMG1_T0LOADHI          (*(volatile uint32_t *)(TIMG1_BASE + 0x1C))
#define TIMG1_INT_ENA_TIMERS    (*(volatile uint32_t *)(TIMG1_BASE + 0x20))
#define TIMG1_INT_RAW           (*(volatile uint32_t *)(TIMG1_BASE + 0x24))
#define TIMG1_INT_CLR           (*(volatile uint32_t *)(TIMG1_BASE + 0x2C))

#define TIMG_T0_EN              (1UL << 31)
#define TIMG_T0_ALARM_EN        (1UL << 30)
#define TIMG_T0_AUTORELOAD      (1UL << 27)
#define TIMG_T0_INCREASE        (1UL << 26)
#define TIMG_T0_DIVIDER(n)      ((n) << 13)

#define APB_CLOCK_HZ            80000000UL
#define TIMER_PRESCALER         80
#define TICK_TIMER_HZ           configTICK_RATE_HZ
#define TIMER_TICKS_PER_TICK    (APB_CLOCK_HZ / TIMER_PRESCALER / TICK_TIMER_HZ)

#define PRO_TG1_T0_LEVEL_INT_MAP (*(volatile uint32_t *)0x3FF41F5CUL)
#define CPU_INTERRUPT_NUM        6

extern volatile uint32_t port_switch_flag;
extern volatile uint32_t port_interrupt_sp;
extern volatile uint32_t port_interrupt_nesting;
extern uint8_t port_isr_stack[];

extern void _frxt_timer_int(void);

#define XT_STK_FRMSZ 0x50
#define XT_STK_PC    0x00
#define XT_STK_PS    0x04
#define XT_STK_A0    0x08
#define XT_STK_A1    0x0C
#define XT_STK_A2    0x10
#define XT_STK_SAR   0x48
#define XT_STK_EXIT  0x4C

typedef struct XtExcFrame {
    uint32_t pc;
    uint32_t ps;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t a8;
    uint32_t a9;
    uint32_t a10;
    uint32_t a11;
    uint32_t a12;
    uint32_t a13;
    uint32_t a14;
    uint32_t a15;
    uint32_t sar;
    uint32_t exit;
} XtExcFrame;

extern volatile uint32_t _vectors_start[];

static void vPortSetupTimerInterrupt(void)
{
    PRO_TG1_T0_LEVEL_INT_MAP = CPU_INTERRUPT_NUM;

    TIMG1_T0HI = 0;
    TIMG1_T0LO = 0;

    uint32_t config = TIMG_T0_EN | TIMG_T0_ALARM_EN | TIMG_T0_AUTORELOAD
                    | TIMG_T0_INCREASE | TIMG_T0_DIVIDER(TIMER_PRESCALER);
    TIMG1_T0CONFIG = config;

    TIMG1_T0ALARMHI = 0;
    TIMG1_T0ALARMLO = TIMER_TICKS_PER_TICK;

    TIMG1_T0LOADLO = 0;
    TIMG1_T0LOADHI = 0;

    TIMG1_INT_ENA_TIMERS = 1;
    TIMG1_INT_CLR = 1;

    __asm__ volatile(
        "rsr     a2, INTENABLE\n"
        "or      a2, a2, %0\n"
        "wsr     a2, INTENABLE\n"
        "rsync\n"
        :: "r"(1UL << CPU_INTERRUPT_NUM) : "a2"
    );
}

void __attribute__((weak)) vPortSetupTimer(void)
{
    vPortSetupTimerInterrupt();
}

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                   TaskFunction_t pxCode,
                                   void *pvParameters)
{
    XtExcFrame *frame;
    uint32_t *sp;
    uint32_t i;

    sp = (uint32_t *)(((uint32_t)pxTopOfStack - XT_STK_FRMSZ) & ~0x0F);

    for (i = 0; i < XT_STK_FRMSZ / 4; i++) {
        sp[i] = 0;
    }

    frame = (XtExcFrame *)sp;

    frame->pc = (uint32_t)pxCode;
    frame->a0 = 0;
    frame->a1 = (uint32_t)sp + XT_STK_FRMSZ;
    frame->a2 = (uint32_t)pvParameters;
    frame->ps = 0x00010;
    frame->exit = 1;

    return sp;
}

BaseType_t xPortStartScheduler(void)
{
    portDISABLE_INTERRUPTS();

    vPortSetupTimer();

    port_interrupt_sp = (uint32_t)port_isr_stack + 2048 - 16;
    port_interrupt_nesting = 0;
    port_switch_flag = 0;

    {
        uint32_t vecbase = (uint32_t)_vectors_start;
        __asm__ volatile(
            "wsr.VECBASE %0\n"
            "rsync\n"
            :: "r"(vecbase)
        );
    }

    __asm__ volatile("call0 _frxt_dispatch\n");

    return pdTRUE;
}

void vPortEndScheduler(void)
{
    for (;;) {}
}

BaseType_t xPortSysTickHandler(void)
{
    TIMG1_INT_CLR = 1;
    return 0;
}

uint32_t xPortGetTickRateHz(void)
{
    return configTICK_RATE_HZ;
}
