MEMORY
{
    iram    (rwx) : ORIGIN = 0x4009E000, LENGTH = 0x1000
    dram    (rw)  : ORIGIN = 0x3FFB0000, LENGTH = 0x10000
}

ENTRY(_start)

SECTIONS
{
    .text : ALIGN(4)
    {
        _text_start = .;
        *(.literal .literal.*)
        *(.text .text.*)
        _text_end = .;
    } > iram

    .rodata : ALIGN(4)
    {
        *(.rodata*)
    } > iram

    .bss (NOLOAD) : ALIGN(16)
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > dram

    _stack_start = ORIGIN(dram) + LENGTH(dram);
}
