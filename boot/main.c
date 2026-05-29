#include <stdint.h>
#include <stddef.h>

#define GPIO_OUT_W1TS_REG      (*(volatile uint32_t *)0x3FF44008)
#define GPIO_OUT_W1TC_REG      (*(volatile uint32_t *)0x3FF4400C)
#define GPIO_ENABLE_W1TS_REG   (*(volatile uint32_t *)0x3FF44024)
#define IO_MUX_GPIO2_REG       (*(volatile uint32_t *)0x3FF49008)
#define IO_MUX_PU              (1u << 8)

typedef int (*spi_read_fn)(uint32_t addr, void *dst, size_t size);

static inline uint32_t read_ccount(void)
{
    uint32_t val;
    __asm__ volatile("rsr.ccount %0" : "=r"(val));
    return val;
}

static void udelay(uint32_t us)
{
    uint32_t start = read_ccount();
    while ((read_ccount() - start) < us * 240) {}
}

static void led_on(void) { GPIO_OUT_W1TS_REG = (1u << 2); }
static void led_off(void) { GPIO_OUT_W1TC_REG = (1u << 2); }

typedef void (*entry_fn)(void);

uint32_t app_buf[8192 / 4];

void boot_main(void)
{
    IO_MUX_GPIO2_REG = IO_MUX_PU;
    GPIO_ENABLE_W1TS_REG = (1u << 2);

    led_on();
    udelay(50000);
    led_off();

    spi_read_fn spi_read = (spi_read_fn)0x40046F50;
    uint32_t app_flash_addr = 0x10000;
    uint8_t *buf = (uint8_t *)app_buf;

    int ret = spi_read(app_flash_addr, buf, 64);

    if (ret != 0) {
        for (;;) { led_on(); udelay(100000); led_off(); udelay(100000); }
    }

    led_on();
    udelay(50000);
    led_off();

    uint32_t hdr = ((uint32_t *)buf)[0];
    uint32_t magic = hdr & 0xFF;

    if (magic != 0xE9) {
        for (;;) { led_on(); udelay(200000); led_off(); udelay(200000); }
    }

    uint32_t nseg = (hdr >> 8) & 0xFF;
    uint32_t entry = ((uint32_t *)buf)[1];

    uint32_t total_size = 24;
    for (uint32_t s = 0; s < nseg; s++) {
        uint32_t seg_hdr[2];
        spi_read(app_flash_addr + total_size, seg_hdr, 8);
        uint32_t addr = seg_hdr[0];
        uint32_t len  = seg_hdr[1];
        total_size += 8;

        uint32_t aligned = (len + 3) & ~3;

        if (len > 0) {
            spi_read(app_flash_addr + total_size, (void *)addr, aligned);
        }
        total_size += aligned;
    }

    led_on();
    udelay(50000);
    led_off();

    uint32_t *sp_dst = (uint32_t *)0x3FFC0000;
    entry_fn fn = (entry_fn)(uint32_t)entry;

    __asm__ volatile(
        "mov a1, %0\n"
        "jx %1\n"
        :: "r"(sp_dst), "r"(fn) : "a1"
    );

    for (;;) {}
}
