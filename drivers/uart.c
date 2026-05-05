#define UART_BASE 0x10000000

// Print a character via the UART MMIO device
int uart_putc(char c){
    // Write the character to the base address of the device
    volatile char *addr = (volatile char *)UART_BASE;
    *addr = c;

    return 0;
}
