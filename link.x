/* OUTPUT_FORMAT(binary) */

MEMORY {
    interrupt_handlers : ORIGIN = (0x100), LENGTH = 0x100
    dos : ORIGIN = (0x100 + 0x100), LENGTH = 0x10000
    framebuffer : ORIGIN = (0x100 + 0x100 + 0x10000), LENGTH = 0x2EE00
}

SECTIONS {
    . = 0x100;
    .interrupt_handlers : { *(.interrupt_handlers); } > interrupt_handlers
    .text : { *(.text); } > dos
    .data : { *(.data); } > dos
    .bss : { *(.bss); } > dos
    .rodata : { *(.rodata); } > dos
    .framebuffer : { *(.framebuffer); } > framebuffer
    _heap = ALIGN(4);
}
