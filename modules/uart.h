#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART0_BASE              0x3FF40000UL
#define UART0_FIFO_REG          (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART0_INT_RAW_REG       (*(volatile uint32_t *)(UART0_BASE + 0x04))
#define UART0_INT_STA_REG       (*(volatile uint32_t *)(UART0_BASE + 0x08))
#define UART0_INT_ENA_REG       (*(volatile uint32_t *)(UART0_BASE + 0x0C))
#define UART0_CONF0_REG         (*(volatile uint32_t *)(UART0_BASE + 0x20))
#define UART0_CONF1_REG         (*(volatile uint32_t *)(UART0_BASE + 0x24))
#define UART0_CLKDIV_REG        (*(volatile uint32_t *)(UART0_BASE + 0x30))
#define UART0_STATUS_REG        (*(volatile uint32_t *)(UART0_BASE + 0x1C))

#define UART_TXFIFO_CNT_M       (0xFFu << 16)
#define UART_TXFIFO_CNT_S       16
#define UART_RXFIFO_CNT_M       (0xFFu << 0)
#define UART_RXFIFO_CNT_S       0
#define UART_TXFIFO_EMPTY       (1u << 11)
#define UART_RXFIFO_FULL        (1u << 10)
#define UART_TXFIFO_FULL        (1u << 9)
#define UART_RXFIFO_EMPTY       (1u << 8)

void uart0_init(uint32_t baud);
void uart0_putc(char c);
void uart0_puts(const char *s);
void uart0_puthex32(uint32_t val);
void uart0_puthex8(uint8_t val);

#endif
