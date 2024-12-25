#ifndef X86_BOOT_H
#define X86_BOOT_H
#include <paging.h>
extern pde_t pagedir[NPDENTRIES];
extern pte_t pagetable[256][NPTENTRIES];
void enable_paging(u32 flag);
#endif