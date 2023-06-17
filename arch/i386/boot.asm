bits 32
KERNEL_BASE_ADDRESS equ 0xC0000000

section .multiboot.text
extern temp_directory
global start
global start_3GB

start:
    cli
    mov ecx, temp_directory
    mov cr3, ecx
    mov ecx, cr4
    or ecx, 0x10
    mov cr4, ecx
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx
    lea ecx, [start_3GB]
    jmp ecx

section .text
extern kernel
global switch_context
global switch_usermode
global get_eip
global isr_common_stub
global irq_common_stub
global farjump
global switch_to_user
global switch_to_task
global _load_gdt
global _load_tss
global _load_idt
global _enable_pg
global _disable_pg
global bios32_helper
global bios32_helper_end
global asm_gdt_ptr
global asm_gdt_entries
global asm_idt_ptr
global asm_in_reg_ptr
global asm_out_reg_ptr
global asm_intnum_ptr
extern new_gdt_entries
extern new_gdt_ptr
extern new_idt_ptr
extern new_reg_ptr
extern new_intnum_ptr

%define REBASE(x) (((x) - bios32_helper) + 0x7c00)
%define GDTENTRY(x) ((x) << 3)
%define CODE32 GDTENTRY(1)
%define DATA32 GDTENTRY(2)
%define CODE16 GDTENTRY(6)
%define DATA16 GDTENTRY(7)

start_3GB:
    mov dword [temp_directory], 0
    invlpg[0]
    mov esp, stack_end
    add ebx, KERNEL_BASE_ADDRESS
	push ebx
	call kernel
.loop:
    jmp .loop

switch_usermode:
    mov bx, 0x23
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    push 0x23
    push esp
    pushfd
    push 0x1B
    lea ebx, [.ret]
    push ebx
    iret
.ret:
    add esp, 8
    ret

switch_context:
    mov eax, [esp + 4]
    mov edx, [esp + 8]
    push ebp
    push ebx
    push esi
    push edi
    mov [eax], esp
    mov esp, [edx]
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

get_eip:
    pop eax
    jmp eax

farjump:
    jmp 0xffff:0
    ret

_load_gdt:
	mov	eax, [esp + 4]
	lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
	jmp	0x08:.flush
.flush:
	ret

_load_idt:
	push ebp
	mov ebp, esp
	mov eax, [ebp + 8]
	lidt [eax]
	pop ebp
    ret

_load_tss:
    mov ax, 0x40
    ltr ax
    ret

bios32_helper: use32
    pusha
    mov ecx, cr0
    and ecx, 0x7fffffff
    mov cr0, ecx
    mov edx, esp
    xor ecx, ecx
    mov ebx, cr3
    mov cr3, ecx
    lgdt [REBASE(asm_gdt_ptr)]
    lidt [REBASE(asm_idt_ptr)]
    jmp CODE16:REBASE(protected_mode_16)
protected_mode_16:use16
    mov ax, DATA16
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, cr0
    and al, ~0x01
    mov cr0, eax
    jmp 0x0:REBASE(real_mode_16)
real_mode_16:use16
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x8c00
    pusha
    mov cx, ss
    push cx
    mov cx, gs
    push cx
    mov cx, fs
    push cx
    mov cx, es
    push cx
    mov cx, ds
    push cx
    pushf
    mov ax, sp
    mov edi, temp_esp
    stosw
    mov esp, REBASE(asm_in_reg_ptr)
    popa
    mov sp, 0x9c00
    db 0xCD
asm_intnum_ptr:
    db 0x000000
    mov esp, REBASE(asm_out_reg_ptr)
    add sp, 28
    pushf
    mov cx, ss
    push cx
    mov cx, gs
    push cx
    mov cx, fs
    push cx
    mov cx, es
    push cx
    mov cx, ds
    push cx
    pusha
    mov esi, temp_esp
    lodsw
    mov sp, ax
    popf
    pop cx
    mov ds, cx
    pop cx
    mov es, cx
    pop cx
    mov fs, cx
    pop cx
    mov gs, cx
    pop cx
    mov ss, cx
    popa
    mov eax, cr0
    inc eax
    mov cr0, eax
    jmp CODE32:REBASE(protected_mode_32)
protected_mode_32:use32
    mov ax, DATA32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov cr3, ebx
    mov esp, edx
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx
    popa
    ret
padding:
    db 0x0
    db 0x0
    db 0x0
asm_gdt_entries:
    resb 72
asm_gdt_ptr:
    dd 0x00000000
    dd 0x00000000
asm_idt_ptr:
    dd 0x00000000
    dd 0x00000000
asm_in_reg_ptr:
    resw 14
asm_out_reg_ptr:
    dd 0xaaaaaaaa
    dd 0xaaaaaaaa
    dd 0xaaaaaaaa
    dd 0xaaaaaaaa
    dd 0xaaaaaaaa
    dd 0xaaaaaaaa
    dd 0xaaaaaaaa
temp_esp:
    dw 0x0000
bios32_helper_end:

section .bss
align 0x1000
stack_start:
    resb 0x1000
stack_end: