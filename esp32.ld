MEMORY
{
    iram    (rwx) : ORIGIN = 0x40080000, LENGTH = 0x20000
    dram    (rw)  : ORIGIN = 0x3FFB0000, LENGTH = 0x10000
}

ENTRY(_start)

SECTIONS
{
    .text : ALIGN(4)
    {
        _text_start = .;
        *(.literal .literal.*)
        KEEP(*(.text._start))
        *(.text .text.*)
        _text_end = .;
    } > iram

    .rodata : ALIGN(4)
    {
        *(.rodata*)
    } > iram

    .data : ALIGN(4)
    {
        _data_start = .;
        *(.data*)
        _data_end = .;
    } > dram AT > iram
    _data_load = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4)
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > dram

    _stack_start = ORIGIN(dram) + LENGTH(dram);
}
