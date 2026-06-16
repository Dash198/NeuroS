#include "kalloc.h"
#include <stdint.h>

#define MAX_MEM 0x88000000

extern char _stack_top[];

struct run *freelist;

void kinit() {
  uint64_t addr = (uint64_t)_stack_top;

  addr = (addr + 4095) & ~4095;

  while (addr < MAX_MEM) {
    struct run *r = (struct run *)addr;
    r->next = freelist;
    freelist = r;

    addr += 4096;
  }
}

void *kalloc() {
  if (!freelist) {
    return (void *)0;
  }

  uint64_t addr = (uint64_t)freelist;

  freelist = freelist->next;

  return (void *)addr;
}

void kfree(void *pa) {
  struct run *r = (struct run *)pa;
  r->next = freelist;
  freelist = r;
}