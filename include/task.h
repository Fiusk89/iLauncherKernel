#pragma once
#ifndef TASK_H
#define TASK_H
#include <kernel.h>
#define TASK_STACK_SIZE 0x1000
#define TASK_RETURN 0x00000000

typedef enum task_state
{
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_TERMINATED,
    TASK_CRASHED,
    TASK_PAUSED,
    TASK_KILLED,
    TASK_EXIT
} task_state_t;

typedef enum task_priority
{
    TASK_PRIORITY_VERY_LOW,
    TASK_PRIORITY_LOW,
    TASK_PRIORITY_NORMAL,
    TASK_PRIORITY_HIGH,
    TASK_PRIORITY_VERY_HIGH,
} task_priority_t;

typedef struct task_register
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} task_register_t;

typedef struct task
{
    uint8_t ready;
    uint8_t name[256];
    uint8_t state;
    uint32_t state_info;
    uint32_t pid;
    uint32_t eip;
    uint32_t active_time;
    struct task_context
    {
        void *stack, *stack1;
        struct page_directory *page;
        struct heap *heap;
    } context;
    uint64_t flags, value;
    struct task *prev;
    struct task *next;
} task_t;

extern task_t *start_task;
extern task_t *current_task;

bool task_node_add(task_t *task);
bool task_node_remove(uint32_t pid);
void task_idle();
task_t *task_get();
void task_remove(task_t *task, uint8_t state, uint32_t state_info, uint32_t value);
void task_fault(register_t *regs);
void schedule();
void task_free(task_t *task);
task_t *task_create(uint8_t *name, void *function, void *flags);
bool task_add(task_t *task);
void task_install();
#endif