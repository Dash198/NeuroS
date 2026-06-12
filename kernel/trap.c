#include <stdint.h>

#include "common.h"
#include "riscv.h"
#include "task.h"
#include "uart.h"

#define MTIME_ADDR 0x0200BFF8
#define MTIMECMP_ADDR 0x02004000
#define INTERVAL 100000

// Define the trap handler enty point
extern void trap_handler(void);

// Global tick counter
volatile uint64_t ticks = 0;

// Trap handler
void handle_trap() {
  // Change the process

  // uart_putc('T');
  ticks++;

  // Update the next comparison
  uint64_t now = *(volatile uint64_t *)MTIME_ADDR;
  *(volatile uint64_t *)MTIMECMP_ADDR = now + INTERVAL;

  current_task->ticks_run++;

  if (ticks % 100 == 0) {
    dump_telemetry();
  }

  sched();
}

void timer_init() {
  // Load the trap handler address into mtvec
  asm volatile("la t0, %0\n\t"
               "csrw mtvec, t0\n\t"
               :
               : "i"(trap_handler));

  // Initialize the comparison reg.
  uint64_t now = *(volatile uint64_t *)MTIME_ADDR;
  *(volatile uint64_t *)MTIMECMP_ADDR = now + INTERVAL;

  // Enable interrupts
  set_mie(1 << 7);
  set_mstatus(1 << 3);
}
