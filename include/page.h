#ifndef PAGE_H
#define PAGE_H
#include <kernel.h>
#define PAGE_SIZE 0x1000
#define BIT_INDEX(a) ((a) / 64)
#define BIT_OFFSET(a) ((a) % 64)

typedef struct page
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t write_through : 1;
    uint32_t cache : 1;
    uint32_t access : 1;
    uint32_t reserved : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t frame : 20;
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
    page_table_t tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddr;
} page_directory_t;

extern page_directory_t *kernel_directory;

void page_enable();
void page_install();
void switch_page_directory(page_directory_t *new);
page_directory_t *clone_directory(page_directory_t *src);
page_t *get_page(uint32_t address, bool make, page_directory_t *dir);
void page_alloc_frame(page_directory_t *page_directory, uint32_t v_address, uint32_t p_address, bool is_kernel, bool is_writeable);
void page_free_frame(page_directory_t *page_directory, uint32_t v_address);
void switch_page_directory(page_directory_t *dir);
#endif