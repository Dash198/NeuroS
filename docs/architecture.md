# 5/5/26

QEMU loads ELF
-> `_start` in `start.S`
-> stack initialised from linker symbol
-> call `kmain`
-> `kmain` calls `uart_putc`
-> UART MMIO writes to `0x10000000`
