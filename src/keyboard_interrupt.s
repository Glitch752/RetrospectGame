# A simple keyboard interrupt handler assigned to IRQ 0x09

.section .text

.code16gcc
.global keyboard_interrupt
keyboard_interrupt:
    pusha                    # Save all general-purpose registers

    # Read the scan code from the keyboard data port (0x60)
    inb $0x60, %al

    # Acknowledge the interrupt by sending an EOI (End of Interrupt) to the PIC
    movb $0x20, %al          # 0x20 is the command to send EOI
    outb %al, $0x20          # Send it to the PIC command port (0x20)

    popa                     # Restore all general-purpose registers
    iret                     # Return from interrupt

.end
