# A simple keyboard interrupt handler assigned to IRQ 0x09

.intel_syntax noprefix
.section .interrupt_handlers
.global keyboard_interrupt
.code16gcc

# This is the entry point of the program
call  _main

mov   ah, 0x4C # 4C is the DOS exit call
int   0x21

keyboard_interrupt:
    pusha                    # Save all general-purpose registers
    
    # Read the scan code from the keyboard data port (0x60)
    inb al, 0x60

    # If the scan code is 0xE0, it's an extended scan code; just ignore it
    cmp al, 0xE0
    je 3f # If al != 0x0E:
        mov bl, al
        and bx, 0x7F # bl is now the real key code (since scan_code & 0x80 indicates if the key was released)
        and al, 0x80
        cmp al, 0 # If the key was pressed
        je 2f # If (scan_code & 0x80) != 0:
            # If the key was released
            movb [KEYS_DOWN + bx], 0
            jmp 3f
        2:
            # If the key was pressed
            movb [KEYS_DOWN + bx], 1
    3:
    # Acknowledge the interrupt by sending an EOI (End of Interrupt) to the PIC
    mov al, 0x20             # 0x20 is the command to send EOI
    outb 0x20, al            # Send it to the PIC command port (0x20)

    popa                     # Restore all general-purpose registers

    iretw

.end
