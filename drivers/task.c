#include <task.h>

typedef uint64_t (*task_function_t)(int32_t argc, int8_t **argv);

extern void isr_exit();
extern uint32_t get_eip();
extern void switch_context(void *old, void *new);
extern uint32_t *page_frames, page_frames_size;

uint32_t task_pause = false;
task_t *start_task, *current_task;
register_t saved_context;
static uint32_t next_pid = 0;

static uint32_t task_pid()
{
    return next_pid++;
}

void task_free(task_t *task)
{
    kfree(task->context.page);
    kfree(task->context.heap);
    kfree(task->context.stack);
    kfree(task);
}

bool task_node_add(task_t *task)
{
    if (!start_task || !task)
        return true;
    task_t *tmp_task = start_task;
    while (tmp_task->next)
        tmp_task = tmp_task->next;
    task->next = (task_t *)NULL;
    task->prev = tmp_task;
    tmp_task->ready = false;
    tmp_task->next = task;
    return false;
}

bool task_node_remove(uint32_t pid)
{
    if (!start_task || !pid)
        return true;
    task_t *tmp_task = start_task;
    while (tmp_task->next->pid != pid && tmp_task->next)
        tmp_task = tmp_task->next;
    if (!tmp_task->next || tmp_task->next->pid != pid)
        return true;
    tmp_task->next->prev->next = tmp_task->next->next;
    tmp_task->next->next->prev = tmp_task->next->prev;
    return false;
}

void task_idle()
{
    while (1)
        asm volatile("hlt");
}

task_t *task_get()
{
    return current_task;
}

void task_remove(task_t *task, uint8_t state, uint32_t state_info, uint32_t value)
{
    if (!task || !task->pid)
        return;
    task_t *node_prev = task->prev;
    node_prev->ready = false;
    task->state = state;
    task->state_info = state_info;
    task->value = value;
    task_node_remove(task->pid);
    if (current_task != task)
        task_free(task);
    node_prev->ready = true;
}

static void task_helper()
{
    task_function_t function = (task_function_t)current_task->eip;
    uint64_t *flags = (uint64_t *)current_task->flags;
    uint64_t value = 0;
    if (!flags)
        value = function(NULL, NULL);
    else
        value = function((int32_t)flags[0], (int8_t **)flags[1]);
    task_remove(current_task, TASK_TERMINATED, NULL, value);
    schedule();
}

task_t *task_create(uint8_t *name, void *function, void *flags)
{
    if (!function || !name)
        return NULL;
    task_t *task = (task_t *)kmalloc(sizeof(task_t));
    memset(task, 0, sizeof(task_t));
    uint32_t name_length = strlen(name);
    if (name_length > 256 - 1)
        memcpy(task->name, name, 255);
    else
        memcpy(task->name, name, name_length);
    task->pid = task_pid();
    task->context.stack = (void *)kmalloc_a(TASK_STACK_SIZE, 0x1000);
    memset(task->context.stack, 0, TASK_STACK_SIZE);
    task_register_t *context = (task_register_t *)task->context.stack;
    context->eip = (uint64_t)isr_exit;
    register_t *regs = (register_t *)((uint64_t)task->context.stack + sizeof(task_register_t));
    regs->eflags = 0x206;
    regs->cs = 0x08;
    regs->gs = regs->fs = regs->es = regs->ds = 0x10;
    regs->eip = (uint32_t)task_helper;
    task->eip = (uint32_t)function;
    task->flags = (uint32_t)flags;
    return task;
}

bool task_add(task_t *task)
{
    if (!task)
        return false;
    task_node_add(task);
    task->prev->ready = true;
    return true;
}

void task_fault(register_t *regs)
{
    if (current_task->pid == 0)
    {
        kprintf("KERNEL ");
        kprintf("TASK CRASHED\n\tPID: %u;\n\tISR: %u;\n", current_task->pid, regs->int_no);
        while (true)
            asm volatile("cli; hlt");
    }
    kprintf("TASK CRASHED\n\tPID: %u;\n\tISR: %u;\n", current_task->pid, regs->int_no);
    task_remove(current_task, TASK_CRASHED, regs->int_no, 0xDEADC0DE);
    schedule();
}

void schedule()
{
    if (!current_task)
    {
        current_task = start_task;
        return;
    }
    task_t *old_task = current_task;
    if (current_task->ready)
    {
        current_task = current_task->next;
        if (current_task->state == TASK_PAUSED)
            current_task = current_task->next;
    }
    else
    {
        current_task = start_task;
    }
    if (!current_task)
        current_task = start_task;
    current_task->state = TASK_RUNNING;
    switch_context(&old_task->context, &current_task->context);
}

void task_install()
{
    current_task = (task_t *)kmalloc(sizeof(task_t));
    memset(current_task, 0, sizeof(task_t));
    current_task->ready = true;
    strcpy(current_task->name, "iLauncherKernel");
    int8_t exceptions[] = {
        0,
        4,
        6,
        8,
        10,
        11,
        12,
        13,
        14,
        -1,
    };
    for (uint8_t i = 0; exceptions[i] != -1; i++)
        isr_add_handler(exceptions[i] ? (uint8_t)exceptions[i] : 0, task_fault);
    start_task = current_task;
}