
AS:=as -am --32
CC:=gcc

CFLAGS:=-ffreestanding -O2 -Wall -Werror -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32 -fplan9-extensions
CPPFLAGS:= -std=gnu99
LIBS:= #-lgcc

OBJS:=\
build/boot.o \
build/io.o \
build/kernel.o \
build/channel.o \
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
build/keyboard.o \
build/lib.o \
build/main.o


all:
	make build/src/; make build/os.iso; make update_img

.PHONY: all clean iso run

build/src/:
	mkdir -p build/src/
	cp sources/*.c build/src/
	cp sources/*.h build/src/
	cp sources/*/*.c build/src/
	cp sources/*/*.h build/src/
	cp sources/*/*.s build/src/

build/os.bin: $(OBJS) sources/boot/linker.ld build/src/
	$(CC) -T sources/boot/linker.ld -o $@ $(CFLAGS) $(OBJS) $(LIBS)

build/%.o: build/src/%.c build build/src/
	$(CC) $< -c -o $@  $(CFLAGS) $(CPPFLAGS)

build/%.o: build/src/%.s build build/src/
	$(AS) $< -o $@

clean:
	rm -rf build
iso: os.iso

build build/isodir build/isodir/boot build/isodir/boot/grub:
	mkdir -p $@

build/isodir/boot/os.bin: build/os.bin build/isodir/boot
	cp $< $@

build/isodir/boot/grub/grub.cfg: sources/boot/grub.cfg build/isodir/boot/grub
	cp $< $@

build/os.iso: build/isodir/boot/os.bin build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ build/isodir

run:
	qemu-system-i386 -boot c -drive format=raw,file=build/disk.img -m 512 -s


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
	sudo rm -rf /mnt/test
	sudo mkdir /mnt/test
	sudo mount /dev/loop1 /mnt/test

umount:
	sudo umount /mnt/test
	sudo rm -rf /mnt/test

grub_inst:
	sudo grub-install --root-directory=/mnt/test --no-floppy --modules="normal part_msdos fat multiboot" /dev/loop0

cp_iso_to_img:
	sudo rsync -r build/isodir/ /mnt/test/

user_programs:build build/isodir
	mkdir -p build/isodir/bin
	./programs/full_build

clean_img: build/disk.img partition load_dev file_syst mount grub_inst umount unload_dev save_img

save_img:
	rm resources/disk.img
	cp build/disk.img resources/disk.img
load_img:
	cp resources/disk.img build/disk.img

update_img: load_img user_programs add_sources load_dev mount cp_iso_to_img umount unload_dev

disk: build clean_img

############# Copy sources to disk image ############
add_sources:
	mkdir -p build/
	mkdir -p build/isodir/
	mkdir -p build/isodir/CacatOS
	cp -R sources/ build/isodir/CacatOS
	mkdir -p build/isodir/CacatOS/programs
	cp -R programs/lib build/isodir/CacatOS/programs/lib
	cp -R programs/src build/isodir/CacatOS/programs/src
