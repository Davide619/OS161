#include <coremap.h>

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <vm.h>
#include <swapfile.h>
#include <pt.h>
#include <addrspace.h>

/* list of free frames (0 -> busy, 1 -> freed) */
//static unsigned char *freeRamFrames = NULL;

/* number of frames allocated from the corresponding frame. 
 * If it is a kernel page knowing this is necessary when
 * the allocated space should be freed.
 */
// static unsigned long *allocSize = NULL;

/* number of frames present inside the RAM */
//static int nRamFrames = 0;

/*pinter to the freeFrameList structure*/
struct ffl * freeFrameList;

/* Set if vm_bootstrap is called */
//static int allocTableActive = 0;

/* spinlock for mutual esclusive access to ram_stealmem */
// static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

// /* spinlock for mutual esclusive access to ram_stealmem */
// static struct spinlock freemem_lock = SPINLOCK_INITIALIZER;



// static int isTableActive () {
//   int active;
//   spinlock_acquire(&freemem_lock);
//   active = allocTableActive;
//   spinlock_release(&freemem_lock);
//   return active;
// }



void vm_bootstrap(void)         /*ADDED*/
{
        // /*Computation of available free frames number in RAM*/
        // int nRamFramesMAX = ((int)ram_getsize())/PAGE_SIZE;
        // //*stack = kmalloc(sizeof(int)*nRamFrames);

        // if(nRamframes > nRamFramesMAX){
        //   kprintf("nRamFrames is higher than nRamFramesMAX. Change the number of Frames!\n");
        //   return;
        // }

        // /*alloco il vettore freeRamFrame inizializzato a 0 (vuoto)*/
        // freeRamFrames = kmalloc(sizeof(unsigned char)*nRamFrames);
        // if (freeRamFrames==NULL) return; 
        
        /*alloco allocSize*/
        // allocSize = kmalloc(sizeof(unsigned long)*nRamFrames);
        // if (allocSize==NULL){
        //         //reset to disable this vm management 
        //         freeRamFrames = NULL; 
        //         return;
        // }

        // /*alloco la struttura FreeFrameList*/
        // as->freeFrameList = ffl_create(nRamFrames);
        // if(as->freeFrameList == NULL){ 
        //   kprintf("FreeFrameList structure is not allocated!\n");
        //   return;
        // }

        // /*inizializzo la struttura*/
        // ffl_init(&freeFrameList, nRamFrames);

        // for (int i=0; i<nRamFrames; i++){
        //         freeRamFrames[i] = (unsigned char)0;
        //         allocSize[i] =0;
        // }
        
        // spinlock_acquire(&freemem_lock);
        // allocTableActive = 1;
        // spinlock_release(&freemem_lock);
}



// static paddr_t 
// getfreeppages(unsigned long npages) {
//   paddr_t addr;	
//   long i, first, found, np = (long)npages;

//   if (!isTableActive()) return 0; 
//   spinlock_acquire(&freemem_lock);
//   for (i=0,first=found=-1; i<nRamFrames; i++) {
//     if (freeRamFrames[i]) {
//       if (i==0 || !freeRamFrames[i-1]) 
//         first = i; /* set first free in an interval */  
//       if (i-first+1 >= np) {
//         found = first;
//         break;
//       }
//     }
//   }
	
//   if (found>=0) {
//     for (i=found; i<found+np; i++) {
//       freeRamFrames[i] = (unsigned char)0;
//     }
//     allocSize[found] = np;
//     addr = (paddr_t) found*PAGE_SIZE;
//   }
//   else {
//     addr = 0;
//   }

//   spinlock_release(&freemem_lock);

//   return addr;
// }
 


// paddr_t
// getppages(unsigned long npages)
// {
//   paddr_t addr;

//   /* try freed pages first */
//   addr = getfreeppages(npages);
//   if (addr == 0) {
//     /* call stealmem */
//     spinlock_acquire(&stealmem_lock);
//     addr = ram_stealmem(npages);
//     spinlock_release(&stealmem_lock);
//   }
//   if (addr!=0 && isTableActive()) {
//     spinlock_acquire(&freemem_lock);
//     allocSize[addr/PAGE_SIZE] = npages;
//     spinlock_release(&freemem_lock);
//   } 

//   return addr;
// }



// static int 
// freeppages(paddr_t addr, unsigned long npages){
//   long i, first, np=(long)npages;	

//   if (!isTableActive()) return 0; 

//   /* compute page index */
//   first = addr/PAGE_SIZE;
//   KASSERT(allocSize!=NULL);
//   KASSERT(nRamFrames>first);

//   /* update freeRamFrame structure in mutual esclusion */ 
//   spinlock_acquire(&freemem_lock);
//   for (i=first; i<first+np; i++) {
//     freeRamFrames[i] = (unsigned char)1;
//   }
//   spinlock_release(&freemem_lock);

//   return 1;
// }



/* Allocate/free some kernel-space virtual pages */
// vaddr_t
// alloc_kpages(unsigned npages)
// {
// 	paddr_t pa;

// 	//vm_can_sleep();
// 	pa = getppages(npages);
// 	if (pa==0) {
// 		return 0;
// 	}
// 	return PADDR_TO_KVADDR(pa);
// }



// void
// free_kpages(vaddr_t addr)
// {
//   /* control that the initialization was performed */
// 	if (isTableActive()) {

//     /* retain paddr from kernel vaddr */
//     	paddr_t paddr = addr - MIPS_KSEG0;

//       /*compute page index*/
//     	long first = paddr/PAGE_SIZE;	
//     	KASSERT(allocSize!=NULL);
//     	KASSERT(nRamFrames>first);
      
//     	freeppages(paddr, allocSize[first]);	
//   	}

// }



/* in order to deallocate frames reserved for the user program,
 * we use start_frame to store the first physical address reserved
 */
static paddr_t start_frame;

struct ffl * ffl_create(const uint8_t nframes)
{
    struct ffl * new_ffl = kmalloc(sizeof(struct ffl) * nframes);
    if(new_ffl == NULL) return NULL;

    for(int idx = 0; idx < nframes; idx++)
    {
        new_ffl[idx].prev = (idx != 0) ? &new_ffl[idx - 1] : NULL;
        new_ffl[idx].next = (idx != nframes - 1) ? &new_ffl[idx + 1] : NULL;
    }

    return new_ffl;
}

void ffl_init(struct ffl ** ffl_init, uint8_t nframes)
{
    paddr_t cur_frame = start_frame = getppages(nframes);

    // initialize to zero the user program frames
    bzero((void *)PADDR_TO_KVADDR(start_frame), nframes * PAGE_SIZE);
    
    (*ffl_init)->free_frame = cur_frame;
    while((*ffl_init)->next != NULL)
    {
        cur_frame += PAGE_SIZE;
        ffl_push(ffl_init, cur_frame);
    } 
    
    return;
}

void ffl_push(struct ffl ** ffl_push, const paddr_t fr_push)
{
    /* first we check the content of free_frame: this is equal to 0 only if
     * the stack is empty, in this case we keep the pointer to the current 
     * ffl structure and proceed to assign fr_push.
     */
    *ffl_push = ((*ffl_push)->free_frame != 0) ? (*ffl_push)->next : *ffl_push;
    (*ffl_push)->free_frame = fr_push;

    return;
}

paddr_t ffl_pop(struct ffl ** ffl_pop)
{
    paddr_t fr_pop = (*ffl_pop)->free_frame;
    (*ffl_pop)->free_frame = 0;

    /* if ffl_pop points to the stack base, we don't want to further decrease
     * ffl_pop: in this case, let's keep the current value of ffl_pop.
     */
    *ffl_pop = ((*ffl_pop)->prev != NULL) ? (*ffl_pop)->prev : *ffl_pop;

    return fr_pop;
}

void ffl_destroy(struct ffl * ffl_dest)
{
    //freeppages(start_frame); da rivedere ma non credo che quello che vuoi fare qui venga fatto da questa funzione 
    kfree(ffl_dest);

    return;
}