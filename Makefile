# Makefile for kacchiOS
# Use the cross-compiler prefixes
CC = i686-elf-gcc
LD = i686-elf-ld
AS = i686-elf-as

CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdinc \
         -fno-builtin -fno-stack-protector -I.
# Note: i686-elf-gcc/as defaults to 32-bit, but you can keep these:
ASFLAGS = --32 
LDFLAGS = -m elf_i386

OBJS = boot.o kernel.o serial.o string.o
OBJS += meminit.o getmem.o freemem.o getstk.o
OBJS += process.o
OBJS += scheduler.o
OBJS+= context_switch.o

all: kernel.elf

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -T link.ld -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

run: kernel.elf
	qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none

run-vga: kernel.elf
	qemu-system-i386 -kernel kernel.elf -m 64M -serial mon:stdio

debug: kernel.elf
	qemu-system-i386 -kernel kernel.elf -m 64M -serial stdio -display none -s -S &
	@echo "Waiting for GDB connection on port 1234..."
	@echo "In another terminal run: gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf'"

clean:
	rm -f *.o kernel.elf

.PHONY: all run run-vga debug clean
