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

/* in order to deallocate frames reserved for the user program,
 * we use start_frame to store the first physical address reserved
 */
static paddr_t start_frame;
static struct ffl * start_ffl;

struct ffl * ffl_create(const uint8_t nframes)
{
    struct ffl * new_ffl = kmalloc(sizeof(struct ffl) * nframes);
    if(new_ffl == NULL) return NULL;

    start_ffl = new_ffl;
    for (int idx = 0; idx < nframes; idx++)
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

void ffl_destroy(void)
{
    freeppages(start_frame);
    kfree(start_ffl);

    return;
}