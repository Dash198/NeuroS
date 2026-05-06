#include <stdint.h>

#define MIE_MTIE 1<<7
#define MSTATUS_MIE 1<<1

// Write the address of the trap handler to the mtvec CSR
static inline void write_mtvec(uint64_t value){
    asm volatile ("csrw mtvec, %0" : : "r"(value));
}

// Set the mstatus bits
static inline void set_mstatus(uint64_t mask){
    asm volatile ("csrs mstatus, %0" : : "r"(mask));
}

// Set the mie bits
static inline void set_mie(uint64_t mask){
    asm volatile ("csrs mie, %0" : : "r"(mask));
}
