C_SOURCES := $(wildcard *.c)\
			 $(wildcard drivers/*.c)\
			 $(wildcard lib/*.c)\
			 $(wildcard fs/*.c)
GCC_FLAGS := -fno-stack-protector\
			 -I include\
			 -g\
			 -c
LD_SOURCES := *.asm_o *.o

default: i386

clean:
	@rm -rf *.o *.asm_o

i386:
	@nasm -felf32 arch/i386/boot.asm -o boot.asm_o
	@nasm -felf32 arch/i386/interrupts.asm -o interrupts.asm_o
	@nasm -felf32 arch/i386/temp_directory.asm -o temp_directory.asm_o
	@nasm -felf32 arch/header.asm -o header.asm_o
	@gcc $(GCC_FLAGS) -m32 $(C_SOURCES)
	@ld -melf_i386 -Tlinker.ld --allow-multiple-definition x64_arithmetics_for_x86.lib $(LD_SOURCES) -o kernel-i386.bin
	@rm -rf *.o *.asm_o
