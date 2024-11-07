ENTRY(_start)

MEMORY {
    SRAM(rx)   : ORIGIN = 0x20000000, LENGTH = 128k
    FLASH(rx)  : ORIGIN = 0x08000000, LENGTH = 512k
}

_mstack = ORIGIN(SRAM) + LENGTH(SRAM);
_pstack = ORIGIN(SRAM) + LENGTH(SRAM)/2;

HIDDEN(__vector_align__ = 128);
HIDDEN(__vector_table_size = (16 + 240) * 4);
HIDDEN(__word_align__ = 4);

SECTIONS
{
    /*
    Make sure vectors are actually 240 
    flash gets aliased to 0x0...0 and the vtable needs to be at the start there
    If we make it a "subsection of rodata, which would make sense?
    Just dont want to overcomplicate it now
    */

    .rodata.vectors : ALIGN(__vector_align__)
    {
        KEEP(*(.rodata.vectors))
    } > FLASH

    ASSERT(ADDR(.rodata.vectors) == ORIGIN(FLASH), "ADDR(.rodata.vectors) != ORIGIN(FLASH)")
    /* ASSERT(SIZEOF(.rodata.vectors) == __vector_table_size, "SIZEOF(.rodata.vectors) != __vector_table_size")*/

    /*
    .isr_vectors : ALIGN(__vector_align__)
    {
        KEEP(*(.rodata._vectors))
    } > FLASH

    ASSERT(ADDR(.vectors) == ORIGN(FLASH), "ADDR(.vectors) != ORIGN(FLASH)");
    ASSERT(SIZEOF(.vectors) == __vector_table_size, "SIZEOF(.vectors) != __vector_table_size");
    */
    
    .rodata : ALIGN(__word_align__)
    {
        /* Get all _.rodata and all _.rodata_ */
        *(.rodata)
        *(.rodata*)
        . = ALIGN(__word_align__);
    } > FLASH

    .init : ALIGN(__word_align__)
    {
        KEEP(*(.init))
        . = ALIGN(__word_align__);
    } > FLASH

    .fini : ALIGN(__word_align__)
    {
        KEEP(*(.fini))
        . = ALIGN(__word_align__);
    } > FLASH

    .text : ALIGN(__word_align__)
    {
        /* Get all _.text and all _.text_ */
        *(.text)
        *(.text*)
        /* is the eh_frame necessary? */
        KEEP(*(.eh_frame))
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
/*        __bss_start__ = __s_bss; /* required for crt0.S */
        *(.bss)
        *(.bss*)
        . = ALIGN(__word_align__);
        __e_bss = .;
 /*       __bss_end__ = __e_bss; /* required for crt0.S */
    } > SRAM

    /* at the end here I need to have some sort of safety check */
}
