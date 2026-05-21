#include <stdint.h>

#include "riscv.h"
#include "uart.h"
#include "common.h"
#include "task.h"

#define MTIME_ADDR 0x0200BFF8
#define MTIMECMP_ADDR 0x02004000
#define INTERVAL 10000000

// Define the trap handler enty point
extern void trap_handler(void);

// Global tick counter
volatile uint64_t ticks = 0;

// Trap handler
void handle_trap(){
    // Change the process

    uart_putc('T');

    // Update the next comparison
    uint64_t now = *(volatile uint64_t *)MTIME_ADDR;
    *(volatile uint64_t *)MTIMECMP_ADDR = now + INTERVAL;

    set_mstatus(1 << 3);

    sched();
    ticks++;


}

void timer_init(){
    // Load the trap handler address into mtvec
    asm volatile (
        "la t0, %0\n\t"
        "csrw mtvec, t0\n\t"
        :
        : "i"(trap_handler)
    );


    // Initialize the comparison reg.
    uint64_t now = *(volatile uint64_t *)MTIME_ADDR;
    *(volatile uint64_t *)MTIMECMP_ADDR = now + INTERVAL;

    // Enable interrupts
    set_mie(1<<7);
    set_mstatus(1<<3);
}
