#include <stdint.h>
#include "uart.h"

void uart0_init(uint32_t baud)
{
    uint32_t clk_div = (80000000UL + (baud >> 1)) / baud;

    UART0_CONF0_REG = (1u << 13) | (3u << 0) | (1u << 25) | (1u << 24);
    UART0_CLKDIV_REG = clk_div;
    UART0_CONF1_REG = 0;
    UART0_CONF0_REG = (1u << 13) | (3u << 0);
    UART0_INT_ENA_REG = 0;
}

void uart0_putc(char c)
{
    while (UART0_STATUS_REG & UART_TXFIFO_FULL) {}
    UART0_FIFO_REG = (uint32_t)(unsigned char)c;
    if (c == '\n') {
        while (UART0_STATUS_REG & UART_TXFIFO_FULL) {}
        UART0_FIFO_REG = (uint32_t)'\r';
    }
}

void uart0_puts(const char *s)
{
    while (*s) {
        uart0_putc(*s++);
    }
}

void uart0_puthex32(uint32_t val)
{
    static const char hex[] = "0123456789ABCDEF";
    uart0_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uart0_putc(hex[(val >> i) & 0xF]);
    }
}

void uart0_puthex8(uint8_t val)
{
    static const char hex[] = "0123456789ABCDEF";
    uart0_putc(hex[(val >> 4) & 0xF]);
    uart0_putc(hex[val & 0xF]);
}
