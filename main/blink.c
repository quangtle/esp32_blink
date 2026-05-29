#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "bt.h"

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

static void raw_putc(char c)
{
    volatile uint32_t *fifo = (volatile uint32_t *)0x3FF40000;
    *fifo = (uint32_t)(unsigned char)c;
    if (c == '\n') *fifo = (uint32_t)'\r';
}

static void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;) {
        GPIO_OUT_W1TS_REG = (1u << 2);
        delay_ms(100);
        GPIO_OUT_W1TC_REG = (1u << 2);
        delay_ms(3900);
    }
}

volatile int _boot_sentinel __attribute__((used)) = 0xDEAD;

int main(void)
{
    IO_MUX_GPIO2_REG = IO_MUX_PU;
    GPIO_ENABLE_W1TS_REG = (1u << 2);

    if (_boot_sentinel == 0xDEAD) { raw_putc('R'); } else { raw_putc('r'); }

    uart0_init(115200);

    raw_putc('U');

    if (bt_init() == 0) raw_putc('i');
    else raw_putc('F');

    if (bt_reset() == 0) raw_putc('r');
    else raw_putc('x');

    raw_putc('\n');

    xTaskCreate(vBlinkTask, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    for (;;) {}
}
