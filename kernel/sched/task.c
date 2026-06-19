#include "task.h"
#include "kalloc.h"
#include "riscv.h"
#include "uart.h"
#include <stdint.h>

context_t scheduler_context;
task_t tasks[MAX_TASKS];
task_t *current_task;

extern volatile uint64_t ticks;
extern uint64_t total_energy;
extern int current_freq;
extern uint64_t swtch(context_t *, context_t *);
uint64_t missed_deadlines = 0;
void runC();
int create_task(void (*entry_point)());

void init() {
  set_mstatus(1 << 3);
  while (1) {
    sleep(500);                       // Sleep for a while
    int task_id = create_task(&runC); // Spawn a new finite task to do work!
    if (task_id != -1) {
      tasks[task_id].deadline_tick = ticks + 40;
    }
  }
}

void runB() {
  set_mstatus(1 << 3);
  while (1) {
    // uart_putc('B');
  }
}

void runA() {
  set_mstatus(1 << 3);
  while (1) {
    // uart_putc('A');
  }
}

void runC() {
  set_mstatus(1 << 3);

  // Do some meaningless heavy math
  volatile int i = 0;
  while (i < 50000000) {
    i++;
  }

  if (ticks > current_task->deadline_tick) {
    missed_deadlines += 1;
  }
  exit();
}

// Need to change this, we'll add priority change mechanism later.
int create_task(void (*entry_point)()) {

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == UNUSED) {
      tasks[i].state = READY;
      tasks[i].task_id = i;
      tasks[i].stack = kalloc();
      tasks[i].context.sp = (uint64_t)tasks[i].stack + 4096;
      tasks[i].context.ra = (uint64_t)entry_point;
      tasks[i].context.s0 = 0;
      tasks[i].context.s1 = 0;
      tasks[i].context.s2 = 0;
      tasks[i].context.s3 = 0;
      tasks[i].context.s4 = 0;
      tasks[i].context.s5 = 0;
      tasks[i].context.s6 = 0;
      tasks[i].context.s7 = 0;
      tasks[i].context.s8 = 0;
      tasks[i].context.s9 = 0;
      tasks[i].context.s10 = 0;
      tasks[i].context.s11 = 0;

      tasks[i].ticks_waiting = 0;
      tasks[i].ticks_run = 0;
      tasks[i].priority = 0;
      tasks[i].priority_ticks = 0;
      tasks[i].wakeup_tick = 0;

      return i;
    }
  }

  return -1;
}

void task_init() {
  create_task(&init);
  create_task(&runA);
  create_task(&runB);
  create_task(&runC);
  current_task = &tasks[0];
}

// Scheduler function, uses round-robing scheduling
void sched() {
  if (current_task->state == RUNNING) {
    current_task->state = READY;
  }
  task_t *old = current_task;
  task_t *next = 0;

  int task_selected = 0;

  for (int p = 0; p < 3; p++) {
    for (int i = current_task->task_id + 1;
         i < current_task->task_id + 1 + MAX_TASKS; i++) {
      int idx = i % MAX_TASKS;

      if (tasks[idx].priority == p && tasks[idx].state == READY &&
          !task_selected) {
        tasks[idx].state = RUNNING;
        next = &tasks[idx];
        task_selected = 1;
      }
    }
  }

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == READY) {
      tasks[i].ticks_waiting++;
    }
  }
  current_task = next;
  swtch(&(old->context), &(current_task->context));
}

void dump_telemetry() {
  uart_puts("--- SYSTEM STATE ---\n");
  uart_puts("Total Energy: ");
  uart_putint(total_energy);
  uart_puts(", Missed Deadlines: ");
  uart_putint(missed_deadlines);
  uart_puts(", Current Freq: ");
  uart_putint(current_freq);
  uart_puts("\n");
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == UNUSED)
      continue;
    uart_puts("Task ID: ");
    uart_putint(tasks[i].task_id);
    uart_puts(", Priority: ");
    uart_putint(tasks[i].priority);
    uart_puts(", Ticks Waited: ");
    uart_putint(tasks[i].ticks_waiting);
    uart_puts(", Ticks Run: ");
    uart_putint(tasks[i].ticks_run);
    uart_puts("\n");
  }
}

void boost_priorities() {
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state != UNUSED) {
      tasks[i].priority = 0;
      tasks[i].priority_ticks = 0;
    }
  }
}

void sleep(int wait_ticks) {
  current_task->state = BLOCKED;
  current_task->wakeup_tick = ticks + wait_ticks;
  sched();
}

void exit() {
  kfree(current_task->stack);
  current_task->state = UNUSED;
  sched();
}