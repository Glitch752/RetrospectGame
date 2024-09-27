ENTRY(_start)

MEMORY {
  dos : org = 0x100, len = (0x15F900 - 0x100) /* 0x15F900 is 1.44MB, the maximum size of a floppy disk */
}

SECTIONS {
  .text   : { *(.startup) *(.text .text.*) }   > dos
  .rodata : { *(.rodata .rodata.*) } > dos
  .data   : { *(.data) }   > dos
  .bss    : { *(.bss) }    > dos
  .stack  : { *(.stack) }  > dos
  _heap = ALIGN(4);
}
