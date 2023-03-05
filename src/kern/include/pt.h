#ifndef _PT_H_
#define _PT_H_

#include <types.h>
#include <lib.h>
#include <vm.h>



/* functions to alloc/dealloc a page table */
paddr_t * pt_create(size_t nentries);
void pt_destroy(paddr_t * pt_dest);

/* functions to Manage page table and replacement*/ 
paddr_t get_victim_frame(paddr_t *pt,uint8_t *entry_valid);
uint8_t get_victim_pt_index(uint8_t *entry_valid);
vaddr_t get_page_number(vaddr_t virtualaddr,uint8_t *entry_valid);
void pt_update(paddr_t *pt,uint8_t *entry_valid,paddr_t new_frame,uint8_t nvalidentries,uint8_t new_pt_index);



#endif /* _PT_H_ */