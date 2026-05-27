#ifndef PORTMACRO_H
#define PORTMACRO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define portCHAR        char
#define portSHORT       short
#define portINT         int
#define portLONG        long
#define portFLOAT       float
#define portDOUBLE      double

typedef uint32_t    StackType_t;
typedef int32_t     BaseType_t;
typedef uint32_t    UBaseType_t;
typedef uint32_t    TickType_t;

#define portMAX_DELAY            ((TickType_t)0xFFFFFFFFUL)
#define portTICK_PERIOD_MS       ((TickType_t)(1000 / configTICK_RATE_HZ))
#define portSTACK_GROWTH         (-1)
#define portBYTE_ALIGNMENT       16
#define portNOP()                __asm__ volatile("nop")
#define portINLINE               __inline__

extern void vPortYield(void);
#define portYIELD()              vPortYield()

extern void vPortYieldFromInt(void);
#define portYIELD_FROM_ISR()     vPortYieldFromInt()

static inline void portDISABLE_INTERRUPTS(void)
{
    __asm__ volatile("rsil a2, 15" ::: "a2");
}

static inline void portENABLE_INTERRUPTS(void)
{
    __asm__ volatile("rsil a2, 0" ::: "a2");
}

static inline UBaseType_t portSET_INTERRUPT_MASK_FROM_ISR(void)
{
    UBaseType_t ret;
    __asm__ volatile("rsil %0, 15" : "=r"(ret));
    return ret;
}

static inline void portCLEAR_INTERRUPT_MASK_FROM_ISR(UBaseType_t level)
{
    __asm__ volatile("wsr.ps %0; rsync" :: "r"(level));
}

#define portENTER_CRITICAL()               portDISABLE_INTERRUPTS()
#define portEXIT_CRITICAL()                portENABLE_INTERRUPTS()
#define portENTER_CRITICAL_FROM_ISR()      portSET_INTERRUPT_MASK_FROM_ISR()
#define portEXIT_CRITICAL_FROM_ISR(x)      portCLEAR_INTERRUPT_MASK_FROM_ISR(x)

#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) void vFunction(void *pvParameters)
#define portTASK_FUNCTION(vFunction, pvParameters) void vFunction(void *pvParameters)

#define portNUM_PROCESSORS          1
#define portUSING_MPU_WRAPPERS      0

#ifdef __cplusplus
}
#endif

#endif
