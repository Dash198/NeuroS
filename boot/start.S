.section .text      # Declare .text as a section
.global _start      # Declare _start as global

_start:
    # For now, just initialize the stack and call kmain
    la sp, _stack_top
    call kmain

# Inf loop on exit
loop:
    j loop
