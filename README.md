# ESP32 Blink (Bare-Metal + FreeRTOS)

Minimal LED blink project for the ESP32 microcontroller using a custom build system with bundled FreeRTOS. No ESP-IDF required.

## Requirements

| Tool | Version | Notes |
|---|---|---|
| **xtensa-esp32-elf toolchain** | GCC 8.x+ | Cross-compiler, assembler, linker for Xtensa LX6 |
| **GNU Make** | 3.81+ | Build automation |
| **Python 3** | 3.6+ | For esptool flash utility |
| **esptool** | 3.x+ | `pip install esptool` |

### Toolchain Installation

**Option A — Espressif IDF Tools (recommended)**
```
pip install idf-tools
idf-tools install xtensa-esp32-elf
idf-tools export
```

**Option B — Manual download**
Download from: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#standard-setup-of-toolchain

## Build & Flash

```bash
make              # compile → build/blink.bin
make flash        # flash to ESP32 on COM4 (edit COM4 in Makefile if needed)
make clean        # remove build artifacts
```

## Project Structure

```
├── Makefile              # Build entry point (custom GNU Make)
├── esp32.ld              # Linker script (IRAM/DRAM layout)
├── main/
│   ├── blink.c           # Application: toggles GPIO2 via FreeRTOS task
│   └── startup.S         # Reset vector, .data/.bss init, calls main()
├── freertos/             # FreeRTOS kernel source (bundled)
│   ├── tasks.c, queue.c, list.c, heap_4.c, timers.c
│   ├── include/          # FreeRTOS public API headers
│   ├── port/             # ESP32 Xtensa LX6 port (call0 ABI)
│   │   ├── port.c        # Tick timer, stack init, scheduler start
│   │   └── portasm.S     # Context switch, interrupt dispatch
│   └── FreeRTOSConfig.h  # Kernel config (1 core, 100 Hz tick)
└── build/                # Output directory
```

## Notes

- **No C library**: Built with `-nostdlib`; `memset`/`memcpy` provided in `freertos/port/port.c`.
- **ABI**: Xtensa `call0` (not windowed). Pass `-mabi=call0` to compiler.
- **Flash config**: DIO, 40 MHz, 4 MB, chip revision 301 (ECO3).
- **Flash address**: `0x1000`.
- **UART**: Hardcoded to `COM4` in the `flash` target.
