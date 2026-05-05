# 3/5/26

Hello, this is my first devlog. I shall start working on the development of NeuroS, an experimental OS as an attempt to integrate ML into OS.

Some details:
- The OS targets a RISCV64 architecture, built on Arch Linux.
- The tools I'm using for now: `riscv64-elf-gcc`, `qemu-system-riscv64`, `riscv64-elf-gdb`
- Today: Set up the repo (done), and set up Makefile.

Alright, I've set up the Makefile, here's what im looking at!

Assembly compilation done using the command:
`riscv64-elf-gcc -march=rv64i -mabi=lp64 -c boot/start.S -o build/start.o`

What each flag means:
- `march`: Target architecture
- `mabi`: Target ABI
- `-c`: Only make the object file, no linking

Then the kernel is compiled using the following command:
`riscv64-elf-gcc -march=rv64i -mabi=lp64 -ffreestanding -nostdlib -c kernel/kmain.c -o build/kmain.o`

Flags:
- `ffreestanding`: No using the standard library

The linking and making the executable is done via:
`riscv64-elf-ld -T boot/linker.ld build/start.o build/kmain.o -o neuros.elf`

# 4/5/26

Day 2. Yesterday we booted the machine. Today..we have the system get into our code.

Ig what i need to do today is populate `start.S`, `kmain.c` and `linker.ld`.

Here's some standard address locations I googled up for RISC-V virtual memory:
- `0x0000_1000` - Boot ROM
- `0x0200_0000` - CLINT (Core local interrupter), for software and timer interrupts
- `0x0C00_0000` - PLIC (Platform Level Interrupt Controller)
- `0x1000_0000` - UART
- `0x1000_1000` - VirtIO MMIO
- `0x8000_0000` - RAM starts from here

On power-up, PC starts at Boot ROM, and then quickly jumps to RAM.

Before running any C code (like `kmain.c`), we must adhere to the RISC-V calling convention and manually init the C runtime

- Initialise `sp`, the stack pointer
- Initialise `gp`, the global pointer
- Explicitly zero out all uninitialised global vars

We then use registers `a0` to `a7` for function args (`int`), can use the `call` psuedo instr.

The `_start` section serves as the low level entry point where the CPU begins execution from, so that's where we do all the clearing ig

Alright, set up all the files. Now NeuroS boots a valid ELF, enters `_start`, initializes stack, and transfers control to `kmain`. Tomorrow, we'll try printing.

# 5/5/26

Day 3!

Today's goal is to get the UART driver to print a message from `kmain`.

UART (Universal Asynchronous Receiver-Transmitter) is a protocol used for serial communication between devices like microcontrollers and sensors

We want to use the UART MMIO device to emit a character into the QEMU terminal by writing a byte into a specific register.

Alright, implemented the first UART MMIO driver on QEMU virt. NeuroS successfully prints!
