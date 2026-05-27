TOOLCHAIN ?= xtensa-esp32-elf
CC        = $(TOOLCHAIN)-gcc
AS        = $(TOOLCHAIN)-as
LD        = $(TOOLCHAIN)-ld
SIZE      = $(TOOLCHAIN)-size

CFLAGS   = -Og -Wall -Wextra -ffunction-sections -fdata-sections -nostdlib -mlongcalls -mabi=call0 -Ifreertos/include -Ifreertos -Ifreertos/port
LDFLAGS  = -T esp32.ld -nostdlib --gc-sections

BUILD_DIR = build

FREERTOS_SRCS = freertos/tasks.c freertos/list.c freertos/queue.c freertos/heap_4.c freertos/port/port.c
FREERTOS_ASMS = freertos/port/portasm.S
SRCS     = main/blink.c $(FREERTOS_SRCS)
ASMS     = main/startup.S $(FREERTOS_ASMS)
OBJS     = $(SRCS:%c=$(BUILD_DIR)/%o) $(ASMS:%S=$(BUILD_DIR)/%o)
TARGET   = blink
ELF      = $(BUILD_DIR)/$(TARGET).elf
BIN      = $(BUILD_DIR)/$(TARGET).bin

.PHONY: all clean flash

all: $(BIN)
	$(SIZE) $(ELF)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) esp32.ld
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BIN): $(ELF)
	python -m esptool --chip esp32 elf2image --flash-mode dio --flash-freq 40m --flash-size 4MB --min-rev-full 301 --max-rev-full 301 --dont-append-digest -o $@ $<

flash: $(BIN)
	python -m esptool --chip esp32 --port COM4 write-flash 0x1000 $<

clean:
	rm -rf $(BUILD_DIR)
