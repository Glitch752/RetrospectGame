static void print(volatile char* string) {
    volatile char* current = string;
    while(*current != '\0') {
        asm volatile(
            "mov $0x0E, %%ah\n"
            "int $0x10\n"
            : /* no output */
            : "a"((0x0E << 8) | *current)
        );
        current++;
    }
}

static void printlong(unsigned long n) {
    volatile char buffer[12];
    int i = sizeof(buffer);
    buffer[--i] = '\0';
    if(n == 0) buffer[--i] = '0';
    else for(; n > 0; n /= 10) buffer[--i] = '0' + (n % 10);
    print((char *) buffer + i);
}

static void print_hex(unsigned long n) {
    volatile char buffer[12];
    int i = sizeof(buffer);
    buffer[--i] = '\0';
    if(n == 0) buffer[--i] = '0';
    else for(; n > 0; n /= 16) {
        int digit = n % 16;
        buffer[--i] = digit < 10 ? '0' + digit : 'A' + digit - 10;
    }
    print("0x");
    print((char *) buffer + i);
}

static void println(volatile char *string) {
    print(string);
    print("\n\r");
}