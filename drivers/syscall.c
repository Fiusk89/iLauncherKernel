#include <syscall.h>

static void *syscall_handlers[] = {
    heap_malloc,
    heap_mrealloc,
    heap_mfree,
};

void syscall_handler(register_t *regs)
{
    if (regs->eax >= sizeof(syscall_handlers) / sizeof(void *))
        return;
    void *function = syscall_handlers[regs->eax];
    uint32_t ret;
    asm volatile("\
        push %1;\
        push %2;\
        push %3;\
        push %4;\
        push %5;\
        call *%6;\
        pop %%ebx;\
        pop %%ebx;\
        pop %%ebx;\
        pop %%ebx;\
        pop %%ebx;\
    "
                 : "=a"(ret)
                 : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(function));
    regs->eax = ret;
}

void syscall_install()
{
    isr_add_handler(0x80, syscall_handler);
}