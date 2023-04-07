#include <pt.h>

static uint8_t index;

/*Allocation of the data structure for the page table*/
paddr_t * pt_create(size_t nentries)
{
    paddr_t * new_pt = kmalloc(sizeof(paddr_t) * nentries);
    if(new_pt == NULL) return NULL;
    
    for(unsigned int i=0; i<nentries; i++)
        new_pt[i]=0;

    return new_pt;
}
/*De-allocation of data structure for the page table*/
void pt_destroy(paddr_t * pt_dest)
{
    kfree(pt_dest);

    return;
}

/*This function uses index to select a victim among entries of page table to replace*/
paddr_t get_victim_frame(paddr_t *pt,uint32_t *entry_valid)
{
        return pt[entry_valid[index]];
}

/*This function allows to extract from entry valid the victim entry of the page table*/
uint32_t get_victim_pt_index(uint32_t *entry_valid)
{
        return entry_valid[index];
}

/*This function allows to extract the page number of a valid page whose virtualaddr belong to*/
vaddr_t get_page_number(vaddr_t virtualaddr,uint32_t *entry_valid)
{
        return ((entry_valid[index]*PAGE_SIZE) + virtualaddr) & PAGE_FRAME;
}

/*This function updates an entry of PAGE TABLE and in the meanwhile an element of ENTRY VALID:
- entry_valid is the vector used to keep track of only the pages loaded in memory(VALID) in order to implement DEMAND PAGING.*/
void pt_update(paddr_t *pt,uint32_t *entry_valid,paddr_t new_frame,uint8_t nvalidentries,uint32_t new_pt_index)
{
    
    entry_valid[index] = new_pt_index;

    pt[entry_valid[index]] = new_frame;
    
    index = ((index + 1) == nvalidentries) ? 0 : (index+1);
    

}