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

  current_task->ticks_run++;
  current_task->priority_ticks++;

  if (ticks % 16 == 0) {
    boost_priorities();
  } else {
    switch (current_task->priority) {
    case 0:
      if (current_task->priority_ticks >= 2) {
        current_task->priority++;
        current_task->priority_ticks = 0;
      }
      break;

    case 1:
      if (current_task->priority_ticks >= 4) {
        current_task->priority++;
        current_task->priority_ticks = 0;
      }
      break;
    }
  }

  if (ticks % 100 == 0) {
    dump_telemetry();
  }

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == BLOCKED && ticks >= tasks[i].wakeup_tick) {
      tasks[i].state = READY;
    }
  }

  // Update the next comparison
  uint64_t now = *(volatile uint64_t *)MTIME_ADDR;
  *(volatile uint64_t *)MTIMECMP_ADDR = now + INTERVAL;

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
