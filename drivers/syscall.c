#include <syscall.h>

static void *syscall_handlers[] = {
    heap_malloc,
    heap_mrealloc,
    heap_mfree,
};

typedef void *(*syscall_function_t)(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e);

void syscall_handler(register_t *regs)
{
    if (regs->eax > sizeof(syscall_handlers) / sizeof(void *) - 1)
        return;
    syscall_function_t function = (syscall_function_t)syscall_handlers[regs->eax];
    regs->eax = (uint64_t)function(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}

void syscall_install()
{
    isr_add_handler(0x80, syscall_handler);
}