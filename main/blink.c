#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#define GPIO_ENABLE_W1TS_REG   (*(volatile uint32_t *)0x3FF44024)
#define GPIO_OUT_W1TS_REG      (*(volatile uint32_t *)0x3FF44008)
#define GPIO_OUT_W1TC_REG      (*(volatile uint32_t *)0x3FF4400C)
#define IO_MUX_GPIO2_REG       (*(volatile uint32_t *)0x3FF49008)
#define IO_MUX_PU              (1u << 8)

static inline uint32_t read_ccount(void)
{
    uint32_t val;
    __asm__ volatile("rsr.ccount %0" : "=r"(val));
    return val;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = read_ccount();
    uint32_t ticks = ms * 1000 * 60;
    while ((read_ccount() - start) < ticks) {}
}

static void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;
    int cycle = 0;

    for (;;) {
        GPIO_OUT_W1TS_REG = (1u << 2);
        delay_ms(500);
        GPIO_OUT_W1TC_REG = (1u << 2);
        delay_ms(500);
        if (++cycle > 9) cycle = 0;
    }
}

int main(void)
{
    IO_MUX_GPIO2_REG = IO_MUX_PU;
    GPIO_ENABLE_W1TS_REG = (1u << 2);

    xTaskCreate(vBlinkTask, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;) {}
}
