ENTRY(_start)

MEMORY {
    SRAM(rx)   : ORIGIN = 0x20000000, LENGTH = 128K
    FLASH(rx)  : ORIGIN = 0x08000000, LENGTH = 512K
}

_mstack = ORIGIN(SRAM) + LENGTH(SRAM);
_pstack = ORIGIN(SRAM) + LENGTH(SRAM)/2;

HIDDEN(__vector_table_size = (1 + 16 + 96) * 4);
HIDDEN(__word_align__ = 4);

SECTIONS
{
    .vectors : ALIGN(__word_align__)
    {
        KEEP(*(.vectors))
        . = ALIGN(__word_align__);
    } > FLASH

    ASSERT(ADDR(.vectors) == ORIGIN(FLASH), "ADDR(.vectors) != ORIGIN(FLASH)")
    ASSERT(SIZEOF(.vectors) == __vector_table_size, "SIZEOF(.vectors) != __vector_table_size")

    .rodata : ALIGN(__word_align__)
    {
        /* Get all _.rodata and all _.rodata_ */
        *(.rodata)
        *(.rodata*)
        . = ALIGN(__word_align__);
    } > FLASH

    .text : ALIGN(__word_align__)
    {
        /* Get all _.text and all _.text_ */
        *(.text)
        *(.text*)
        . = ALIGN(__word_align__);
    } > FLASH

    __si_data = LOADADDR(.data);

    .data : ALIGN(__word_align__)
    {
        __s_data = .;
        *(.data)
        *(.data*)
        . = ALIGN(__word_align__);
        __e_data = .;
    } >SRAM AT> FLASH

    .bss : ALIGN(__word_align__)
    {
        __s_bss = .;
        *(.bss)
        *(.bss*)
        . = ALIGN(__word_align__);
        __e_bss = .;
    } > SRAM

    end = .;
    _end = end;
}
