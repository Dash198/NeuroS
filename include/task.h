#include <stdint.h>

#define MAX_TASKS 64
#define STACK_SIZE 4096

enum task_state { UNUSED, READY, BLOCKED, RUNNING, ZOMBIE };

// Trapframe
typedef struct trapframe_t {

  // General Purpose registers
  uint64_t ra;
  uint64_t sp;
  uint64_t gp;
  uint64_t tp;

  uint64_t t0;
  uint64_t t1;
  uint64_t t2;

  uint64_t s0;
  uint64_t s1;

  uint64_t a0;
  uint64_t a1;
  uint64_t a2;
  uint64_t a3;
  uint64_t a4;
  uint64_t a5;
  uint64_t a6;
  uint64_t a7;

  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;

  uint64_t t3;
  uint64_t t4;
  uint64_t t5;
  uint64_t t6;

  uint64_t mepc;
  uint64_t mstatus;
} trapframe_t;

// Save all the calle-saved registers
typedef struct context_t {
  // Stack pointer
  uint64_t ra;
  uint64_t sp;
  // Frame pointer
  uint64_t s0;
  // Saved registers
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
} context_t;

typedef struct task_t {
  uint64_t task_id;
  enum task_state state;

  context_t context;

  void *stack;

  uint64_t ticks_run;
  uint64_t ticks_waiting;
  int priority_ticks;
  int wakeup_tick;
  uint64_t deadline_tick;
  uint64_t workload_size;

  int priority;
} task_t;

extern context_t scheduler_context;
extern task_t tasks[MAX_TASKS];
extern task_t *current_task;

void initAB();
void runB();
void runA();
void task_init();
void sched();
void dump_telemetry();
void boost_priorities();
void sleep(int);
void exit();
