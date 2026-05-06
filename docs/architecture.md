# 5/5/26

QEMU loads ELF
-> `_start` in `start.S`
-> stack initialised from linker symbol
-> call `kmain`
-> `kmain` calls `uart_putc`
-> UART MMIO writes to `0x10000000`

# 6/6/26

QEMU Boot
 -> _start
 -> kmain
 -> timer_init
 -> idle loop

[Periodic Hardware Interrupt]
 -> trap_handler (assembly)
 -> handle_trap (C)
 -> UART debug / tick update
 -> reset mtimecmp
 -> mret
