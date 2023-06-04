bits 32
section .multiboot.data
align 0x0008
multiboot_start:
    dd 0x1BADB002
    dd 0x00000000
    dd -0x1BADB002
multiboot_end:

align 0x0008
multiboot2_start:
    dd 0xe85250d6
    dd 0x00000000
    dd multiboot2_end - multiboot2_start
    dd -(0xe85250d6 + 0 + (multiboot2_end - multiboot2_start))
    dw 0x00000000
    dw 0x00000000
    dd 0x00000008
multiboot2_end: