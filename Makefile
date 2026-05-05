CC = riscv64-elf-gcc
LD = riscv64-elf-ld

CFLAGS = -march=rv64i -mabi=lp64 -ffreestanding -nostdlib -c
LDFLAGS = -T boot/linker.ld
INCFLAGS = -I./include

all: build_dir build/neuros.elf

build_dir:
	mkdir -p build

build/neuros.elf: build/start.o build/kmain.o build/uart.o
	$(LD) $(LDFLAGS) build/start.o build/uart.o build/kmain.o -o build/neuros.elf

build/start.o: boot/start.S
	$(CC) -march=rv64i -mabi=lp64 -c boot/start.S -o build/start.o

build/kmain.o: kernel/kmain.c
	$(CC) $(CFLAGS) $(INCFLAGS) kernel/kmain.c -o build/kmain.o

build/uart.o: drivers/uart.c
	$(CC) $(CFLAGS) $(INCFLAGS) drivers/uart.c -o build/uart.o

clean:
	rm -rf build neuros.elf

qemu: all
	qemu-system-riscv64 -machine virt -bios none -kernel build/neuros.elf -nographic
