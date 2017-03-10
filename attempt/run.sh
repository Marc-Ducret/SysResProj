as boot.s -o boot.o --32
gcc -c linker.ld kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32
gcc -T linker.ld -o myos.bin -ffreestanding -O2 -Wall -Wextra -nostdlib -nostartfiles -nodefaultlibs -m32 boot.o kernel.o
make run-qemu

