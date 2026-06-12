CC = riscv64-elf-gcc
LD = riscv64-elf-ld

CFLAGS = -march=rv64im_zicsr -mabi=lp64 -ffreestanding -nostdlib -msmall-data-limit=0 -mcmodel=medany -c
LDFLAGS = -T boot/linker.ld
INCFLAGS = -I./include

all: build_dir build/neuros.elf

build_dir:
	mkdir -p build

build/neuros.elf: build/start.o build/trap_entry.o build/trap.o build/kmain.o build/uart.o build/swtch.o build/task.o
	$(LD) $(LDFLAGS) build/start.o build/trap_entry.o build/trap.o build/uart.o build/task.o build/swtch.o build/kmain.o -o build/neuros.elf

build/start.o: boot/start.S
	$(CC) -march=rv64im_zicsr -mabi=lp64 -c boot/start.S -o build/start.o

build/trap_entry.o: boot/trap.S
	$(CC) -march=rv64im_zicsr -mabi=lp64 -c boot/trap.S -o build/trap_entry.o

build/trap.o: kernel/trap.c
	$(CC) $(CFLAGS) $(INCFLAGS) kernel/trap.c -o build/trap.o

build/swtch.o: kernel/sched/swtch.S
	$(CC) -march=rv64im_zicsr -mabi=lp64 -c kernel/sched/swtch.S -o build/swtch.o

build/task.o: kernel/sched/task.c
	$(CC) $(CFLAGS) $(INCFLAGS) kernel/sched/task.c -o build/task.o

build/kmain.o: kernel/kmain.c
	$(CC) $(CFLAGS) $(INCFLAGS) kernel/kmain.c -o build/kmain.o

build/uart.o: drivers/uart.c
	$(CC) $(CFLAGS) $(INCFLAGS) drivers/uart.c -o build/uart.o

clean:
	rm -rf build neuros.elf

qemu: all
	qemu-system-riscv64 -machine virt -bios none -kernel build/neuros.elf -nographic
