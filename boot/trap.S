.section .text
.global trap_handler

trap_handler:
    addi sp, sp, -8
    sd ra, 0(sp)
    call handle_trap
    ld ra, 0(sp)
    addi sp, sp, 8
    mret
