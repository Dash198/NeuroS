struct run {
  struct run *next;
};

extern struct run *freelist;

void kinit();
void *kalloc();
void kfree(void *);