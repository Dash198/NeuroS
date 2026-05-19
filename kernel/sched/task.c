#include "task.h"
#include "uart.h"
#include <stdint.h>

context_t *scheduler_context;
task_t tasks[MAX_TASKS];
task_t *current_task;

uint8_t stackA[STACK_SIZE];
uint8_t stackB[STACK_SIZE];

extern uint64_t swtch(context_t *, context_t *);

void runB() {
  while (1) {
    uart_putc('B');
  }
}

void runA() {
  while (1) {
    uart_putc('A');
  }
}

void initAB() {
  uart_putc('I');
  tasks[0].task_id = 0;
  tasks[0].state = RUNNING;
  tasks[0].context.sp = (uint64_t)&stackA[STACK_SIZE];
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
  tasks[1].context.sp = (uint64_t)&stackB[STACK_SIZE];
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

void task_init() {
  initAB();
  scheduler_context = &tasks[1].context;
}

// Scheduler function to be implemented later
void sched() {
  task_t *next;
  task_t *old = current_task;
  if (current_task == &tasks[0]) {
    next = &tasks[1];
  } else {
    next = &tasks[0];
  }

  current_task = next;
  swtch(&(old->context), &(current_task->context));
}
