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

# 6/6/26

Day 4

Today we want to add a time machine and implement timer interrupts in our OS. Essentially we add traps!

Things that need to be understood:
- `mtvec` register, CSR instructions
- Role of CLINT, `mtime` and `mtimecmp` addresses
- Machine timer interrupt bits
- How a RISC-V machine trap handler looks like

Address number `0x305` corresponds to `mtvec`, so we need to write there.
In itself, `mtvec` is a CSR that holds the memory address of the trap handle routine, CPU must jump to this address to handle the event.

CLINT is a hw compnent responsible for manging timer and software interrupts for individual processor cores. `mtime` is a 64-bit real time counter and `mtimecmp` is a compare register.

In a QEMU virtual machine:
- The CLINT base address is `0x02000000`
- `mtime` address: `0x0200BFF8`
- `mtimecmp` address: `0x02004000`

The `mie` register at address `0x304` is the machine interrupt-enable register, the `mie.MTIE` bit (`7`) must be enabled. Bit `5` corresponds to `mia.STIE`. Machine mode is the highest privilege level, Supervisor is intermediate, and User is lowest.

The `mstatus` register (`0x300`) register has some config options, and to enable interrupts in machine and supervisor mode, the bits `1` and `3` must be set.

rn we do not have to worry about supervisor mode. NeuroS is currently sunlge hart, machine mode only, direct machine trap handling.

Okay, big day! NeuroS now supports periodic machine timer interrupts.

Lots of assembly stuff, took a whole to understand. but now the kernel fires timer interrupts asynchronously!

# 8/6/26

ran into some errors:
- Firstly the `ticks` variable was in the `.sbss` section, and the linker tried to use an optimization that isn't compatible wih our memory laout. Specifically, the compiler put `ticks` in `.sbss` but the linker cannot find a valid way to address it within the allowed range. So we had to add the flag `-msmall-data-limit=0` to stop the compiler form using `.sbss` entirely
  - After this, `ticks` went to `.bss` but the linker still had a problem. This is because in RISC-V, even standard absolute addressing ahs a distance limit of 2GB from instruction to data. If the linker places them very far apart, the standard code fails. We had to add `-mcmodel=medany` which tells the compiler to use "medium any" addressing, which allows the code and data to be placed anywhere in the 64-bit address space.

Now, the actual Day 5:
> building the first scheuler/task abstraction layer

No context switching/processes, we just want to introduce it. Need to add task representation, scheduler stae, ownership structures.

# 9/6/26

Day 6, huh?

So yesterday we made a small struct to represent a task. Today we think about the logistics that our OS has to take care of when switching tasks.

Time to study `xv6` lol.

Okay so context switching is done vua the `swtch` routine in the file `swtch.S`.

Now onto the RISC-V calling conventions:
- Caller Saved Regs: `x1`, `x5-7`, `x10-17`, `x28-31`
- Callee Saved Regs: `x2`, `x8-9`, `x18-27`

There are some other things in `xv6` like process parent, process size, pagetabl etc which idk if they're relevant to me rn.

Future process of switching:
- Save task context `context_t` into memory, load the `context_t` of the new task.
- Need to make an array of `task_t` to load their metadata.

`xv6` handles scheduling quite gracefully since theirs is round-robing, so they can go on an inf loop and be happy. My MLFQ was built on top of that same loop, so ig we could follow a similar type of scheduling loop for now.

# 12/6/26

Day 7, then. Today we start to approach actual multitasking

Today we'lll build conceptual and structural machinery for the first runnable kernel tasks. New tasks do not have their registers initialized but the machine expects them to be. So we need to work on that.

We need to pre-construct the tsk context before the task even runs because the code expects that.

So we'll just create some dummy tasks and illustrate multitasking.

things that need to happen:
- Each task needs independent stack memory
- A task must possess a valid initial context before first switch
- Initial `ra` determines first execution entrypoint
- `swtch` only manipulates saved contexts

`xv6` has a special first process named `init` that solves the context problem, cuz for the tasks to be scheduled, we need some context!

For now we'll only create `taskA` and `taskB` and alternate bw them.

alright got it, ran into a bug where `swtch` expected `ra` and then `sp` but i accidentally defined them in the inverse order ;-;
Works now tho

# 16/6/26

Got busy with hiring for Epoch. Time to get back! Today we shall make the switching timer-interrupt based rather than voluntary.
