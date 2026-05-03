CC = riscv64-elf-gcc
LD = riscv64-elf-ld

CFLAGS = -march=rv64i -mabi=lp64 -ffreestanding -nostdlib -c
LDFLAGS = -T boot/linker.ld

all: build_dir neuros.elf

build_dir:
	mkdir -p build

neuros.elf: build/start.o build/kmain.o
	$(LD) $(LDFLAGS) build/start.o build/kmain.o -o neuros.elf

build/start.o: boot/start.S
	$(CC) -march=rv64i -mabi=lp64 -c boot/start.S -o build/start.o

build/kmain.o: kernel/kmain.c
	$(CC) $(CFLAGS) kernel/kmain.c -o build/kmain.o

clean:
	rm -rf build neuros.elf

qemu: all
	qemu-system-riscv64 -machine virt -bios none -kernel neuros.elf -nographic
