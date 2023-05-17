////////////////////////////////////////////////////////////
//
// swap
//

/*
 * swapfile operations in swap.c:
 *
 * swap_bootstrap:   bootstraps the swapfile and completes VM-system 
 *                   bootstrap. Declared in vm.h.
 *
 * swap_shutdown:    closes the swapfile vnode. Declared in vm.h.
 * 
 * swap_alloc:       finds a free swap page and marks it as used.
 *                   A page should have been previously reserved.
 *
 * swap_free:        unmarks a swap page.
 *
 * swap_pagein:      Reads a page from the requested swap address 
 *                   into the requested physical page.
 *
 * swap_pageout:     Writes a page to the requested swap address 
 *                   from the requested physical page.
 */

#define SWAP_READ 0
#define SWAP_WRITE 1

void        swap_bootstrap(void);
off_t	 	swap_alloc(vaddr_t vaddr);
void 		swap_free(off_t swapaddr);
void        swap_clear(void);
off_t       search_swapped_frame(vaddr_t vaddr);

int		swap_pagein(vaddr_t vaddr, off_t swapaddr);
int		swap_pageout(vaddr_t vaddr, off_t swapaddr);

void        swap_shutdown(void);

/*
 * Special disk address:
 *   INVALID_SWAPADDR is an invalid swap address.
 */
#define INVALID_SWAPADDR	(0)
#define SWAP_SIZE 9437184 /*Byte      9M*1024B*1024B*/


//extern struct lock *global_paging_lock;

typedef struct st_t{
    vaddr_t addr[SWAP_SIZE / PAGE_SIZE];
    off_t offset_swapfile[SWAP_SIZE / PAGE_SIZE];
}swappage_trace;

////////////////////////////////////////////////////////////