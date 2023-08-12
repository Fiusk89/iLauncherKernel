C_SOURCES := $(wildcard *.c)\
			 $(wildcard drivers/*.c)\
			 $(wildcard lib/*.c)\
			 $(wildcard fs/*.c)
CC := clang
CC_FLAGS := -fno-stack-protector\
			-I include\
			-w\
			-c
LD_SOURCES := *.lib *.asm_o *.o
QEMU_FLAGS := -machine pc\
			  -vga cirrus\
			  -usb\
			  -device usb-kbd\
			  -m 128M

default: i386-build

clean:
	@rm -rf *.o *.asm_o

i386-build:
	@nasm -felf32 arch/i386/boot.asm -o boot.asm_o
	@nasm -felf32 arch/i386/interrupts.asm -o interrupts.asm_o
	@nasm -felf32 arch/i386/temp_directory.asm -o temp_directory.asm_o
	@nasm -felf32 arch/header.asm -o header.asm_o
	@$(CC) $(CC_FLAGS) -m32 $(C_SOURCES)
	@ld -melf_i386 -Tlinker.ld --allow-multiple-definition $(LD_SOURCES) -o kernel-i386.bin
	@rm -rf *.o *.asm_o

i386-emu:
	@kvm $(QEMU_FLAGS) -kernel kernel-i386.bin