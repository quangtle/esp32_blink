#include <stdint.h>

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

static void delay_us(uint32_t us)
{
    uint32_t start = read_ccount();
    uint32_t ticks = us * (read_ccount() > 200000000 ? 240U : 160U);
    while ((read_ccount() - start) < ticks) {}
}

int main(void)
{
    /* Init GPIO2 */
    IO_MUX_GPIO2_REG = IO_MUX_PU;
    GPIO_ENABLE_W1TS_REG = (1u << 2);

    /* Blink 5 times fast to signal we reached main() */
    for (int i = 0; i < 5; i++) {
        GPIO_OUT_W1TS_REG = (1u << 2);
        delay_us(100000);
        GPIO_OUT_W1TC_REG = (1u << 2);
        delay_us(100000);
    }

    /* Slow blink */
    for (;;) {
        GPIO_OUT_W1TS_REG = (1u << 2);
        delay_us(500000);
        GPIO_OUT_W1TC_REG = (1u << 2);
        delay_us(500000);
    }
}
