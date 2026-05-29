TOOLCHAIN ?= xtensa-esp32-elf
CC        = $(TOOLCHAIN)-gcc
AS        = $(TOOLCHAIN)-as
LD        = $(TOOLCHAIN)-ld
SIZE      = $(TOOLCHAIN)-size

# Use python if available (Windows), otherwise use python3 (macOS/Linux)
PYTHON ?= $(shell which python 2>/dev/null || which python3 2>/dev/null || echo python)

# Auto-detect serial port across platforms
PORT ?= $(shell \
	if [ -e /dev/tty.usbserial* ] 2>/dev/null; then \
		ls /dev/tty.usbserial* 2>/dev/null | head -1; \
	elif [ -e /dev/ttyUSB* ] 2>/dev/null; then \
		ls /dev/ttyUSB* 2>/dev/null | head -1; \
	elif [ -e /dev/ttyACM* ] 2>/dev/null; then \
		ls /dev/ttyACM* 2>/dev/null | head -1; \
	else \
		echo "COM4"; \
	fi)

CFLAGS   = -Os -Wall -Wextra -ffunction-sections -fdata-sections -nostdlib -mlongcalls -mabi=call0 -Ifreertos/include -Ifreertos -Ifreertos/port -Imodules
LDFLAGS  = -T esp32.ld -nostdlib --gc-sections

BUILD_DIR = build

FREERTOS_SRCS = freertos/tasks.c freertos/list.c freertos/queue.c freertos/heap_4.c freertos/port/port.c
FREERTOS_ASMS = freertos/port/portasm.S
SRCS     = main/blink.c modules/bt.c modules/uart.c $(FREERTOS_SRCS)
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
	$(PYTHON) -m esptool --chip esp32 elf2image --flash_mode dio --flash_freq 40m --flash_size 4MB --min-rev-full 301 --max-rev-full 301 --dont-append-digest -o $@ $<

flash: $(BIN)
	$(PYTHON) -m esptool --chip esp32 --port $(PORT) write_flash 0x1000 $<

clean:
	rm -rf $(BUILD_DIR)
