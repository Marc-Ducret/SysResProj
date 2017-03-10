as --32 boot.s -o boot.o
gcc -c kernel.c linker.ld -o kernel.o -ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32
gcc -T linker.ld -o myos.bin -ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32 boot.o kernel.o -lgcc
make run-qemu

