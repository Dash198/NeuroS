#include "common.h"
#include "riscv.h"
#include "task.h"
#include "uart.h"
#include <stdint.h>

#define MTIME_ADDR 0x0200BFF8
#define MTIMECMP_ADDR 0x02004000
#define INTERVAL 100000

// Define the trap handler enty point
extern void trap_handler(void);

// Global tick counter
volatile uint64_t ticks = 0;
extern uint64_t missed_deadlines;
// Simulate DVFS
int current_freq = 1;

uint64_t total_energy = 0;

int count_ready_tasks() {
  int ready_tasks = 0;

  // Count how many tasks want the CPU right now
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == READY || tasks[i].state == RUNNING) {
      ready_tasks++;
    }
  }

  return ready_tasks;
}

void ondemand_governor() {

  int ready_tasks = count_ready_tasks();
  // Classic Threshold Heuristic
  if (ready_tasks >= 3) {
    current_freq = 3; // Panic! Go to MAX speed
  } else if (ready_tasks == 2) {
    current_freq = 2; // Moderate work
  } else {
    current_freq = 1; // Only 1 task running, save energy
  }
}

int previous_error = 0;
int integral = 0;

void pid_governor() {
  int ready_tasks = count_ready_tasks(); // (Write a helper for this)
  int target_tasks = 1;                  // We ideally only want 1 task running

  int error = ready_tasks - target_tasks;
  integral = integral + error;
  int derivative = error - previous_error;

  // Integer tuning constants (Kp, Ki, Kd)
  int output = (10 * error) + (2 * integral) + (5 * derivative);

  if (output > 20)
    current_freq = 3;
  else if (output > 10)
    current_freq = 2;
  else
    current_freq = 1;

  previous_error = error;
}

void oracle_governor() {
  int max_freq_needed = 1;

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == READY || tasks[i].state == RUNNING) {
      // We only care about worker tasks with actual deadlines!
      if (tasks[i].workload_size > 0 && tasks[i].deadline_tick > ticks) {

        uint64_t ticks_remaining = tasks[i].deadline_tick - ticks;
        if (ticks_remaining == 0)
          ticks_remaining = 1; // Prevent div by zero

        // How much work MUST we finish per tick to meet the deadline?
        uint64_t work_per_tick = tasks[i].workload_size / ticks_remaining;

        int required_freq = 1;
        if (work_per_tick > 3200000) {
          required_freq = 3; // We need Freq 3 to survive!
        } else if (work_per_tick > 1600000) {
          required_freq = 2; // We need Freq 2 to survive!
        }

        // If this task demands a higher frequency, bump up the global freq!
        if (required_freq > max_freq_needed) {
          max_freq_needed = required_freq;
        }
      }
    }
  }

  current_freq = max_freq_needed;
}

// Trap handler
void handle_trap() {
  // Change the process

  // uart_putc('T');
  ticks++;
  total_energy += (current_freq * current_freq);
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

  oracle_governor();
  // Update the next comparison
  uint64_t now = *(volatile uint64_t *)MTIME_ADDR;
  *(volatile uint64_t *)MTIMECMP_ADDR = now + current_freq * INTERVAL;

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
  *(volatile uint64_t *)MTIMECMP_ADDR = now + current_freq * INTERVAL;

  // Enable interrupts
  set_mie(1 << 7);
  set_mstatus(1 << 3);
}
