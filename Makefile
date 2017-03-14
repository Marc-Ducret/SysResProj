AS:=as --32
CC:=gcc

CFLAGS:=-ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32
CPPFLAGS:= -std=gnu99
LIBS:= #-lgcc

OBJS:=\
build/boot.o \
build/io.o \
build/kernel.o \
build/printing.o \
build/main.o


all: build/os.bin

.PHONEY: all clean iso run-qemu

build/os.bin: $(OBJS) sources/linker.ld
	$(CC) -T sources/linker.ld -o $@ $(CFLAGS) $(OBJS) $(LIBS)

build/%.o: sources/%.c build
	$(CC) $< -c -o $@  $(CFLAGS) $(CPPFLAGS)

build/%.o: sources/%.s build
	$(AS) $< -o $@

clean:
	rm -rf build
	
iso: os.iso

build build/isodir build/isodir/boot build/isodir/boot/grub:
	mkdir -p $@

build/isodir/boot/os.bin: build/os.bin build/isodir/boot
	cp $< $@

build/isodir/boot/grub/grub.cfg: sources/grub.cfg build/isodir/boot/grub
	cp $< $@

build/os.iso: build/isodir/boot/os.bin build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ build/isodir

run-qemu: build/os.iso
	qemu-system-i386 -cdrom build/os.iso
