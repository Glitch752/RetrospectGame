/* OUTPUT_FORMAT(binary) */

MEMORY {
    dos : ORIGIN = 0x0100, LENGTH = 0x10000
    framebuffer : ORIGIN = (0x0100 + 0x10000), LENGTH = 0x1F400
}

SECTIONS {
    . = 0x0100;
    .text : { *(.text); } > dos
    .data : { *(.data); } > dos
    .bss : { *(.bss); } > dos
    .rodata : { *(.rodata); } > dos
    .framebuffer : { *(.framebuffer); } > framebuffer
    _heap = ALIGN(4);
}
