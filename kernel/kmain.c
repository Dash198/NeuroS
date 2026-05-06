#include "uart.h"
#include "common.h"

int kmain(){
    // Initial print
    uart_putc('N');
    uart_putc('e');
    uart_putc('u');
    uart_putc('r');
    uart_putc('o');
    uart_putc('S');
    // Init timer
    timer_init();

    // Inf loop
    for(int i=0; ; i++){}
    return 0;
}
