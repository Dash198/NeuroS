#include <stdint.h>
#define UART_BASE 0x10000000

// Manual integer-to-string conversion
void int_to_string(uint64_t num, char *str) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Print a character via the UART MMIO device
int uart_putc(char c){
    // Write the character to the base address of the device
    volatile char *addr = (volatile char *)UART_BASE;
    *addr = c;

    return 0;
}

int uart_puts(char *str){
    for(int i=0; str[i]!='\0'; i++){
        uart_putc(str[i]);
    }

    return 0;
}

int uart_putint(uint64_t num){
    char str[32];
    int_to_string(num, str);
    uart_puts(str);
}
