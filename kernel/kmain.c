#include "uart.h"
#include "task.h"
#include "common.h"

extern uint64_t swtch(context_t*, context_t*);

int kmain(){
    // Initial print
    uart_putc('N');
    uart_putc('e');
    uart_putc('u');
    uart_putc('r');
    uart_putc('o');
    uart_putc('S');
    // Init timer
    //timer_init();

    initAB();
    uart_putc('S');
    swtch(&scheduler_context, &tasks[0].context);
    uart_putc('S');
    // Inf loop
    for(int i=0; ; i++){}
    return 0;
}
