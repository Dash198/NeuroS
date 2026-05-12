#include <stdint.h>

#define MAX_TASKS 2
#define STACK_SIZE 4096

enum task_state { READY, BLOCKED, RUNNING, ZOMBIE, UNUSED };

// Save all the calle-saved registers
typedef struct context_t{
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
} task_t;

extern context_t scheduler_context;
extern task_t tasks[MAX_TASKS];

void initAB();
void runB();
void runA();
