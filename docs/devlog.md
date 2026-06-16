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

# 19/6/26

Alright, got it working!

Some things:

- Had to first initialize the timer and comparison registers before enabling interrupts
- Global interrupts are disabled on a timer interrupt so had to re-init them on an interrupt
- had to update the cmp reg before calling `sched` otherwise it wouldnt work.
- Now we move on to making a proper task model and scheduler

# 21/6/26

instead of dedicated functions to declare `A` and `B`, made a gneeral function `create_task`.

Faced some silent loops due to the following:
- The `.bss` section says everything placed there like uninitialized global variables must be initialized to 0.
- The `enum task_state` was initially ordered such that `READY` was corresponding to 0, so all the tasks were assigned `READY`.
- Hence we couldn't find a task that was `UNUSED`!

also built a round robin scheduler!

# 12/6/26 - The Return & The Bug Hunt!

After coming back from a break, we tackled three massive OS bugs and learned some incredible systems lessons today!

### 1. The Missing M-Extension (Compiler vs Bare Metal)
When trying to do division or modulo (`%`) on 64-bit integers, the linker threw an `undefined reference to '__umoddi3'` error.
**The Lesson:** Our Makefile was compiling with `-march=rv64i_zicsr` (base integer instructions only). Because we didn't include the `M` (Multiplication/Division) extension, the compiler assumed the CPU couldn't do hardware division and tried to call a software division function from the standard library (`libgcc`). Since we compile with `-nostdlib` (bare metal), it crashed! 
**The Fix:** We changed `-march=rv64i_zicsr` to `-march=rv64im_zicsr` to tell the compiler we have hardware division.

### 2. The Context Switch Illusion (`ticks++`)
When we started dumping telemetry, it dumped exactly 3 times and then stopped dumping, but continued running.
**The Lesson:** In our `handle_trap` function, `ticks++` was placed *after* the `sched()` context switch. When a new task runs for the very first time, it jumps directly to its entry point (like `runB`) and *never returns* to the rest of the interrupt handler! So `ticks++` was skipped. It only started incrementing normally once all 3 tasks had been fully initialized and resumed from `sched()`.
**The Fix:** We moved `ticks++;` to the top of `handle_trap` above the `sched()` call!

### 3. Stack Overflow via Nested Interrupts
Task 1 and Task 2 completely stopped getting scheduled, leaving Task 0 to run forever.
**The Lesson:** We enabled interrupts (`set_mstatus(1<<3)`) inside the interrupt handler `handle_trap` itself! Since we lowered the timer `INTERVAL` to 1000 cycles, the timer fired again *while* we were busy printing telemetry to the slow UART. This caused the trap handler to recursively push 264-byte trapframes onto Task 0's stack until it overflowed its 4KB limit and corrupted the adjacent `tasks` array memory in `.bss`. This effectively deleted Task 1 and Task 2's states!
**The Fix:** Never enable interrupts inside an interrupt handler! Instead, the tasks must enable their own interrupts when they start running (or we set `MPIE` in the trapframe before `mret`).

# 14/6/26

### 4. Interrupt Storms and Recursive Faults
Even after fixing the nested interrupt stack overflow, the OS completely froze, and QEMU terminated unexpectedly. Task 1 ran thousands of times while Task 0 and Task 2 stopped entirely.
**The Lesson:** Our `INTERVAL` was set to `1000` cycles. However, `dump_telemetry()` takes tens of thousands of cycles to print text over the simulated UART. Because the interval was so short, by the time the trap handler finished, the next timer interrupt was *already* pending and fired immediately upon returning to the task (`mret`). The tasks were effectively getting 0 execution time, creating an **Interrupt Storm**. Eventually, this slight memory corruption caused the scheduler to not find any `READY` tasks, leading it to dereference a NULL `current_task` pointer. This caused a recursive fault loop that completely crashed the QEMU machine.
**The Fix:** We increased the `INTERVAL` back to `100000`, allowing the trap handler plenty of time to finish and giving the tasks their fair CPU timeslice.

### 5. Priority Scheduling and Starvation
We implemented our first Strict Priority Scheduler with 3 queues. However, upon booting, only Task A (priority 0) ever ran!
**The Lesson:** This wasn't a bug in the code; it was a perfect demonstration of **Starvation**! In strict priority scheduling, if the highest priority task never yields or blocks for I/O (like our infinite `while(1)` loops), the scheduler will never look at lower priority tasks. They simply starve to death. This perfectly highlights why modern operating systems do not use strict priorities without mitigation.
**Next Steps:** We are going to evolve this strict priority scheduler into a **Multi-Level Feedback Queue (MLFQ)** to implement "Aging" (preventing starvation by boosting priorities over time) and "Demotion" (punishing CPU-heavy tasks).

### 6. MLFQ and The Harmonic Resonance Bug
We implemented the MLFQ with Time Allotments (demotion after running for X ticks) and Aging (boosting all tasks to priority 0 every 16 ticks). However, the telemetry showed that Task 2 was completely stuck at Priority 2, while Task 0 and Task 1 flawlessly bounced between Priority 0 and 1! 
**The Lesson:** This was a mathematically perfect edge case—a **Harmonic Resonance**! Our `boost_priorities()` function only boosted tasks in the `READY` state. Because Task 0 and Task 1 always consumed exactly 12 ticks combined before falling to priority 2, Task 2 would begin its execution right before the 16th tick. When the 16-tick boost hit, Task 2 was currently `RUNNING`. Since it wasn't `READY`, it missed the priority boost and stayed at priority 2 forever! The cycle of Task 0 and 1 combined with the 16-tick interval perfectly synchronized so that Task 2 was *always* the one running when the boost occurred.
**The Fix:** We changed the boost condition from `tasks[i].state == READY` to `tasks[i].state != UNUSED`, ensuring that even the currently running task gets its priority refreshed. The MLFQ is now mathematically perfect!

### 7. Physical Memory Allocation (June 16, 2026)
Before we can introduce dynamic task lifecycles (tasks spawning and exiting), the OS needs a way to hand out memory. We finally replaced our static `stacks` array with a true **Page Allocator**.
**The Architecture:** We used the linker script's `_stack_top` symbol to find where the kernel code ends, and `0x88000000` as the hardware limit of our QEMU RAM. We chopped this free space into 4096-byte pages.
**The Implementation:** Instead of keeping an external array to track memory, we implemented a classic **Free List**. The bookkeeping data `struct run { struct run *next; }` is stored *inside* the free pages themselves! 
**Gotchas Encountered:**
1. **Prepending vs Appending:** We initially tried to append to the tail of the free list, which caused a NULL pointer dereference. We learned that OS memory allocators don't care about order, so it's safer and cleaner to just prepend to the head.
2. **Stack Math (Growing Downwards):** When we updated `create_task()` to use `kalloc()`, we accidentally set the Stack Pointer (`sp`) to the bottom of the allocated page. Since RISC-V stacks grow downwards, pushing to the stack instantly corrupted the page below it! We fixed this by adding `4096` to the pointer returned by `kalloc()`.
**Result:** Phase 2 is complete! Tasks are now running flawlessly on dynamically allocated stacks.
