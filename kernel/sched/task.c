#include "task.h"
#include "riscv.h"
#include "uart.h"
#include <stdint.h>

context_t scheduler_context;
task_t tasks[MAX_TASKS];
task_t *current_task;

uint8_t stacks[MAX_TASKS][STACK_SIZE];

extern uint64_t swtch(context_t *, context_t *);

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
  while (1) {
    // uart_putc('C');
  }
}

void initAB() {
  uart_putc('I');
  tasks[0].task_id = 0;
  tasks[0].state = RUNNING;
  // tasks[0].context.sp = (uint64_t)&stackA[STACK_SIZE];
  tasks[0].context.ra = (uint64_t)runA;
  tasks[0].context.s0 = 0;
  tasks[0].context.s1 = 0;
  tasks[0].context.s2 = 0;
  tasks[0].context.s3 = 0;
  tasks[0].context.s4 = 0;
  tasks[0].context.s5 = 0;
  tasks[0].context.s6 = 0;
  tasks[0].context.s7 = 0;
  tasks[0].context.s8 = 0;
  tasks[0].context.s9 = 0;
  tasks[0].context.s10 = 0;
  tasks[0].context.s11 = 0;

  tasks[1].task_id = 1;
  tasks[1].state = READY;
  // tasks[1].context.sp = (uint64_t)&stackB[STACK_SIZE];
  tasks[1].context.ra = (uint64_t)runB;
  tasks[1].context.s0 = 0;
  tasks[1].context.s1 = 0;
  tasks[1].context.s2 = 0;
  tasks[1].context.s3 = 0;
  tasks[1].context.s4 = 0;
  tasks[1].context.s5 = 0;
  tasks[1].context.s6 = 0;
  tasks[1].context.s7 = 0;
  tasks[1].context.s8 = 0;
  tasks[1].context.s9 = 0;
  tasks[1].context.s10 = 0;
  tasks[1].context.s11 = 0;
}

// Need to change this, we'll add priority change mechanism later.
int create_task(void (*entry_point)(), int priority) {

  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == UNUSED) {
      tasks[i].state = READY;
      tasks[i].task_id = i;
      tasks[i].context.sp = (uint64_t)&stacks[i][STACK_SIZE];
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
      tasks[i].priority = priority;

      return i;
    }
  }

  return -1;
}

void task_init() {
  create_task(&runA, 0);
  create_task(&runB, 1);
  create_task(&runC, 2);
  current_task = &tasks[0];
}

// Scheduler function, uses round-robing scheduling
void sched() {
  current_task->state = READY;
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
      } else if (p == 0 && tasks[idx].state == READY) {
        tasks[idx].ticks_waiting++;
      }
    }
  }

  current_task = next;
  swtch(&(old->context), &(current_task->context));
}

void dump_telemetry() {
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == UNUSED)
      continue;
    uart_puts("Task ID: ");
    uart_putint(tasks[i].task_id);
    uart_puts(", Ticks Waited: ");
    uart_putint(tasks[i].ticks_waiting);
    uart_puts(", Ticks Run: ");
    uart_putint(tasks[i].ticks_run);
    uart_puts("\n");
  }
}
