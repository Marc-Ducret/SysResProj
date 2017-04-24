
AS:=as -am --32
CC:=gcc

CFLAGS:=-ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32 -fplan9-extensions
CPPFLAGS:= -std=gnu99
LIBS:= #-lgcc

OBJS:=\
build/boot.o \
build/io.o \
build/kernel.o \
build/printing.o \
build/memory.o \
build/paging.o \
build/timer.o \
build/int_handlers.o \
build/isr.o \
build/gdt.o \
build/disk.o \
build/partition.o \
build/filesystem.o \
build/file_name.o \
build/fs_call.o \
build/stream.o \
build/error.o \
build/keycode.o \
build/keyboard.o \
build/shell.o \
build/syscall.o \
build/lib.o \
build/main.o


all: build/os.bin

.PHONY: all clean iso run-qemu-disk

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

iso-run-qemu: build/os.iso
	qemu-system-i386 -cdrom build/os.iso

run-qemu-disk: build/os.iso update_img
	qemu-system-i386 -boot c -drive format=raw,file=build/disk.img -m 512 -s

debugbochs: build/os.iso update_img
	bochs


####### DISK IMAGE #########

build/disk.img:
	dd if=/dev/zero of=build/disk.img bs=512 count=131072

partition: build/disk.img
	sudo echo "," | sfdisk build/disk.img

load_dev: build/disk.img
	sudo losetup /dev/loop0 build/disk.img
	sudo losetup /dev/loop1 build/disk.img -o 1048576

unload_dev:
	sudo losetup -d /dev/loop0
	sudo losetup -d /dev/loop1

file_syst:
	sudo mkdosfs -F32 -f 2 /dev/loop1

mount:
	sudo mkdir /mnt/test
	sudo mount /dev/loop1 /mnt/test

umount:
	sudo umount /mnt/test
	sudo rm -rf /mnt/test

grub_inst:
	sudo grub-install --root-directory=/mnt/test --no-floppy --modules="normal part_msdos fat multiboot" /dev/loop0

cp_iso_to_img:
	sudo rsync -r build/isodir/ /mnt/test/

clean_img: clean build/os.iso build/disk.img partition load_dev file_syst mount grub_inst umount unload_dev save_img

save_img:
	rm resources/disk.img
	cp build/disk.img resources/disk.img
load_img:
	cp resources/disk.img build/disk.img

update_img: load_img build/os.iso load_dev mount cp_iso_to_img umount unload_dev
