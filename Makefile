TOOLCHAIN ?= xtensa-esp32-elf
CC        = $(TOOLCHAIN)-gcc
AS        = $(TOOLCHAIN)-as
LD        = $(TOOLCHAIN)-ld
SIZE      = $(TOOLCHAIN)-size

CFLAGS   = -Og -Wall -Wextra -ffunction-sections -fdata-sections -nostdlib -mlongcalls -mabi=call0
LDFLAGS  = -T esp32.ld -nostdlib --gc-sections

SRCS     = main/blink.c
ASMS     = main/startup.S
OBJS     = $(SRCS:.c=.o) $(ASMS:.S=.o)
TARGET   = blink

.PHONY: all clean flash

all: $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJS) esp32.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).bin: $(TARGET).elf
	python -m esptool --chip esp32 elf2image --flash-mode dio --flash-freq 40m --flash-size 4MB --min-rev-full 301 --max-rev-full 301 --dont-append-digest -o $@ $<

flash: $(TARGET).bin
	python -m esptool --chip esp32 --port COM4 write-flash 0x1000 $<

clean:
	rm -f main/*.o *.elf *.bin *.map
