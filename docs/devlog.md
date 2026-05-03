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
