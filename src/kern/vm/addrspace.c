/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *      The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <current.h>

/*ADDED*/
#include <pt.h>
#include <vm_tlb.h> //<-----Da controllare dove si trova vm.h (mips)
#include <swapfile.h>
#include <coremap.h>
#include <segments.h>

///////////////////////////
#include <syscall.h>
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <mips/tlb.h>
#include <elf.h>
#include <vnode.h>
#include <uio.h>
#include <vfs.h>
#include <test.h>
//////////////////////////

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM      <------- IMPORTANTE      
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */



struct addrspace *
as_create(void)
{
        struct addrspace *as;

        as = kmalloc(sizeof(struct addrspace));
        if (as == NULL) {
                return NULL;
        }

        /*
         * Initialize as needed.
         */
        as->as_vbase1 = 0;
        as->as_npages2 = 0;             /*CAPIRE SE SERVE vbase/npage2*/
        as->as_vbase2 = 0;
        as->as_npages1 = 0;

        /*PT*/
        as -> pt = NULL;

        as -> code_seg_start = 0;
        as -> code_seg_offset =0;
        as -> code_seg_size =0;
        as -> npagesCode =0;

        as -> data_seg_start = 0;
        as -> data_seg_offset =0;
        as -> data_seg_size =0;
        as -> npagesData =0;

        as -> as_frames = 0;

        as->padd_stack =0;

        /*freeframes list*/
        as->freeFrameList = 0;

        /*Valid entry*/
        as->entry_valid = NULL;

        return as;
}


static
void vm_can_sleep(void){

        if (CURCPU_EXISTS()) {
                /* must not hold spinlocks */
                KASSERT(curcpu->c_spinlocks == 0);

                /* must not be in an interrupt handler */
                KASSERT(curthread->t_in_interrupt == 0);
        }
}



void
as_destroy(struct addrspace *as)
{
        /*
         * Clean up as needed.
         */
        vm_can_sleep();

        kfree(as);
}



void
as_activate(void)
{
        struct addrspace *as;
        int spl;

        as = proc_getas();
        if (as == NULL) {
                /*
                 * Kernel thread without an address space; leave the
                 * prior address space in place.
                 */
                return;
        }

        /*
         * Write this.
         */
        /* Disable interrupts on this CPU while frobbing the TLB. */
        /*TLB invalidation*/
        spl = splhigh();

        for(int i=0; i<NUM_TLB; i++){
                tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
        }

        splx(spl);
}



void
as_deactivate(void)
{
        /*
         * Write this. For many designs it won't need to actually do
         * anything. See proc.c for an explanation of why it (might)
         * be needed.
         */
}



/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
// int
// as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
// 		 int readable, int writeable, int executable)
// {
// 	size_t npages;

// 	dumbvm_can_sleep();

// 	/* Align the region. First, the base... */
// 	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
// 	vaddr &= PAGE_FRAME;

// 	/* ...and now the length. */
// 	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

// 	npages = sz / PAGE_SIZE;

// 	/* We don't use these - all pages are read-write */
// 	(void)readable;
// 	(void)writeable;
// 	(void)executable;

// 	if (as->as_vbase1 == 0) {
// 		as->as_vbase1 = vaddr;
// 		as->as_npages1 = npages;
// 		return 0;
// 	}

// 	if (as->as_vbase2 == 0) {
// 		as->as_vbase2 = vaddr;
// 		as->as_npages2 = npages;
// 		return 0;
// 	}

// 	/*
// 	 * Support for more than two regions is not available.
// 	 */
// 	kprintf("dumbvm: Warning: too many regions\n");
// 	return ENOSYS;
// }
 


int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize, off_t offset, size_t filesize,
                 int readable, int writeable, int executable)
{
        /*
         * Write this.
         */
        size_t npages;

        vm_can_sleep();

        /* Align the region. First, the base... */
        memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
        vaddr &= PAGE_FRAME;

        /* ...and now the length. */
        memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;

        npages = memsize / PAGE_SIZE;

        /* We don't use these - all pages are read-write */
        (void)readable;
        (void)executable;

        if (!writeable) {
                /*if it is not writable the segment is code segment*/
                /*i save its parameters*/
                as -> code_seg_start = vaddr;
                as -> code_seg_offset = offset;
                as -> code_seg_size = filesize;
                as-> npagesCode = npages;

        }else{
                /*data segment*/
                as -> data_seg_start = vaddr;
                as -> data_seg_offset = offset;
                as -> data_seg_size = filesize;
                as-> npagesData = npages;

        }

        if (as->as_vbase1 == 0) {
                as->as_vbase1 = vaddr;
                as->as_npages1 = npages;                                
                return 0;
        }
        if (as->as_vbase2 == 0) {               /*questa parte forse non serve*/
                as->as_vbase2 = vaddr;
                as->as_npages2 = npages;
                return 0;
        }

        /*
         * Support for more than two regions is not available.
         */
        kprintf("Warning: too many regions\n");
        return ENOSYS;                                                                 

}



static void as_zero_region(paddr_t paddr, unsigned npages)
{
        bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}



// int
// as_prepare_load(struct addrspace *as)
// {
// 	KASSERT(as->as_pbase1 == 0);
// 	KASSERT(as->as_pbase2 == 0);
// 	KASSERT(as->as_stackpbase == 0);

// 	dumbvm_can_sleep();

// 	as->as_pbase1 = getppages(as->as_npages1);
// 	if (as->as_pbase1 == 0) {
// 		return ENOMEM;
// 	}

// 	as->as_pbase2 = getppages(as->as_npages2);
// 	if (as->as_pbase2 == 0) {
// 		return ENOMEM;
// 	}

// 	as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
// 	if (as->as_stackpbase == 0) {
// 		return ENOMEM;
// 	}

// 	as_zero_region(as->as_pbase1, as->as_npages1);
// 	as_zero_region(as->as_pbase2, as->as_npages2);
// 	as_zero_region(as->as_stackpbase, DUMBVM_STACKPAGES);

// 	return 0;
// }



int
as_prepare_load2(struct addrspace *as)
{

        //Kassert termina il programma solo se la condizione nella parentesi è falsa
        KASSERT(as->npagesCode != 0);
        KASSERT(as->npagesData != 0);

        vm_can_sleep();

        /*PT initialization (CODE + DATA)*/
        as -> pt = pt_create(as->npagesCode + as->npagesData);
        if(as -> pt == NULL){
                panic("PT DATA and CODE entries not allocated. The program is stopping...\n");
                return ENOMEM; 
        }

        /*FFL initialization*/
        /*alloco la struttura FreeFrameList*/
        as->freeFrameList = ffl_create(NFRAMES);
        if(as->freeFrameList == NULL){ 
          kprintf("FreeFrameList structure is not allocated!\n");
          return ENOMEM;
        }
        /*inizializzo la struttura*/
        ffl_init(&(as->freeFrameList), NFRAMES);

        /* Initialize the stack */
        as->padd_stack = getppages(STACKPAGES);
        if(as->padd_stack == 0){
                kprintf("Warning: Stack allocation failed\n");
                return ENOMEM;
        }
        as_zero_region(as->padd_stack, STACKPAGES);

        // /*Frames allocations*/
        // as->as_frames = getppages(NFRAMES);     //alloco un numero fisso di frames
        // if (as->as_frames == 0) {
        //         kprintf("Warning: Frames allocation failed\n");
        //         return ENOMEM;          
        // }


        //as_zero_region(as->as_frames, NFRAMES);

        return 0;
}

int
as_complete_load(struct addrspace *as)
{
        /*
         * Write this.
         */
        vm_can_sleep();
        (void)as;
        return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	KASSERT(as->padd_stack != 0);

	*stackptr = USERSTACK;
	return 0;
}






int vm_fault(int faulttype, vaddr_t faultaddress)                        
{
        vaddr_t vbase1, stackbase, stacktop;
        paddr_t frame_number, old_frame, stack_padd;
        int ret_value, ret_TLB_value, flagRWX, load_from_elf;
	uint8_t pt_index,old_pt_index;
        struct addrspace *as;
        off_t index_swapfile, page_in_swapfile, off_fromELF;
        
        DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

        /*check if the fault is due to attempt text section modification*/
        if(faulttype == VM_FAULT_READONLY){
		kprintf("Attempt to modify the text section.\nExiting...\n");
		sys__exit(0);
	}       
	
        if (curproc == NULL) {
                /*
                 * No process. This is probably a kernel fault early
                 * in boot. Return EFAULT so as to panic instead of
                 * getting into an infinite faulting loop.
                 */
                return EFAULT;
        }

        as = proc_getas(); //Acquisisco l'address space dell'attuale processo che dopo andrò a controllare con le KASSERT

        if (as == NULL) {
                /*
                 * No address space set up. This is probably also a
                 * kernel fault early in boot.
                 */
                return EFAULT; 
        }

	
	KASSERT((as->as_vbase1 != 0) & ((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1)); // <---modified(solo reso compatto)
	KASSERT((as->code_seg_start != 0) & ((as->code_seg_start & PAGE_FRAME) == as->code_seg_start));
	KASSERT((as->data_seg_start != 0) & ((as->data_seg_start & PAGE_FRAME) == as->data_seg_start));
	
        vbase1 = as->as_vbase1;	/*indirizzo virtuale d'inizio di un segmento che viene definito nella funzione as_define_region*/
				/*tale segmento può essere di dato o codice dipende dall'ELFile*/

	
        stackbase = USERSTACK - STACKPAGES * PAGE_SIZE;
        stacktop = USERSTACK;
	
	/*se il faultaddress appartiene allo stack, aggiorno solo la TLB e salto tutto il codice sottostante*/
	if (faultaddress >= stackbase && faultaddress < stacktop) {
                stack_padd = (faultaddress - stackbase) + as->padd_stack; /*stack*/
		
		/* make sure it's page-aligned */
        	KASSERT((stack_padd & PAGE_FRAME) == stack_padd);
		
		ret_TLB_value = tlb_insert(stack_padd, 0, 1,faultaddress);
		return 0;
        }
	
	
 	/*getting the page number*/
        //page_number = faultaddress & PAGE_FRAME;                
        
        /*getting the offset*/
        //page_offset = faultaddress & 0x00000fff;                       
	
	/*index in PT*/
	pt_index = (faultaddress-vbase1)/PAGE_SIZE;
	
	/*check for an invalid entry in PT*/
	if(as->pt[pt_index] == 0){
		
		index_swapfile = search_swapped_frame(faultaddress); /*La funzione vuole come parametro l'indirizzo virtuale della pagina
									da cercare nello swapfile. Ritorna 0 se non trova nulla oppure l'indice*/
		/*Check if the searching was successful*/
		if(index_swapfile ==0){
			load_from_elf = 1; /*frame out from swap file*/			/*<------variable to be declered (global or static)?*/
		}
		
		/*pop from freeframelist*/
		frame_number = ffl_pop(&(as->freeFrameList)); 
		
		/*check if freeframelist is empty*/
		if(frame_number == 0){	/*free frame list vuoto*/
			/*replacement algorithm*/
			/*assumiamo che si debba lavorare con indirizzi virtuali e che le informazioni 
			vengano automaticamente caricate in memoria fisica (RAM)*/
			old_frame = get_victim_frame(as->pt, as->entry_valid);
			old_pt_index = get_victim_pt_index(as->entry_valid);
			
			/*swap out*/
			page_in_swapfile = swap_alloc(get_page_number(vbase1,as->entry_valid)); //supponendo vbase1 indirizzo virtuale di partenza del primo segmento nell'elf
												/*La funzione "swap_alloc" vuole come parametro l'indirizzo virtuale della pagina da allocare nello swap*/
			ret_value = swap_pageout(get_page_number(vbase1,as->entry_valid), page_in_swapfile);/*La funzione "swap_pageout" vuole:
													PRIMO parametro --> l'indirizzo virtuale della pagina da portare nello swap file
													SECONDO parametro --> offset corrispondente alla posizione di allocazione della pagina nello swap File*/
			if(ret_value == 0){
				kprintf("Swap_out page is DONE!\n");
			}else{
				panic("ERROR swap_out page! the program is stopping...\n");
			}
			
			/*PT update*/
			as->pt[old_pt_index] = 0;
			pt_update(as->pt, as->entry_valid, old_frame, NFRAMES, pt_index);
			
			
			/*TLB update*/
			/*carico la TLB con la nuova entry*/
			
			
			ret_TLB_value = tlb_insert(old_frame, frame_number, 1,faultaddress); /*questa funzione chiama automaticamente TLBreplace se non trova spazio*/
											/*PRIMO parametro --> indirizzo fisico della pagina che si vuole inserire
											 SECONDO parametro --> indirizzo fisico della pagina che si vuole inserire
											 TERZO parametro --> 0/1 decide quale dei due indirizzi fisici prendere
											 QUARTO parametro --> indirizzo virtuale corrispondente a quella pagina fisica*/
			if (ret_TLB_value == 0){
				kprintf("TLB was not FULL, new TLB entry is loaded!\n");
			}else{
				kprintf("TLB was FULL, new TLB entry is loaded by REPLACEMENT ALGORITHM!\n");
			}
			
			

			/* Clear page of memory to be replaced */
			bzero((void*)(faultaddress & PAGE_FRAME), PAGE_SIZE);				
			
					
					
			/*Checking where the frame has to be loaded from*/
			if(load_from_elf == 1){
				
				/*check if the faultaddress comes from DATA seg, CODE seg*/
                                switch(which_segment(as,faultaddress)){
                                        case CODE_SEG:
                                                off_fromELF = offset_fromELF(as->code_seg_start, faultaddress, (as -> code_seg_size), (as -> code_seg_offset));
					        flagRWX = 0;
                                        break;
                                        case DATA_SEG:
                                                off_fromELF = offset_fromELF(as->data_seg_start, faultaddress, (as -> data_seg_size), (as -> data_seg_offset));
					        flagRWX = 1;
                                        break;
                                        case ERR_SEG:
                                                panic("INVALID faultaddress. The program is stopping...\n");
                                        break;
                                        default:
                                                panic("INVALID faultaddress. The program is stopping...\n");
                                                flagRWX =0;
                                                off_fromELF =0;
                                }
				
					
				/*i will load the frame from elf_file*/
				ret_value = load_page_fromElf(off_fromELF, faultaddress, PAGE_SIZE, PAGE_SIZE, flagRWX);			
				if(ret_value ==0){
					kprintf("Frame is loaded from elfFile!\n");
				}else{
					panic("ERROR load from ElfFile! the program is stopping...\n");
				}
			}else{
				/*i will load the frame from swap_file*/
				index_swapfile = search_swapped_frame(faultaddress); /*La funzione vuole come parametro l'indirizzo virtuale della pagina
											da cercare nello swapfile*/
				/*Check if the searching was successful*/
				if(index_swapfile ==0){
					panic("ERROR search_swapped_frame function was NO successful! the program is stopping...\n");
				}
				
				ret_value = swap_pagein(get_page_number(vbase1,as->entry_valid), index_swapfile);	/*Starting virtualaddress in memory as first parameter of the function
														(where i expect to find the virtual address that corresponds to physical one)*/
														/*La funzione "swap_pagein" vuole:
													PRIMO parametro --> l'indirizzo virtuale della pagina da portare in memoria
													SECONDO parametro --> Posizione (offset) corrispondente alla relativa pagina nello swap File*/
				if(ret_value ==0){
					kprintf("Swap_in page from swap_file is DONE!\n");
				}else{
					panic("ERROR swap_in page from swap_file! the program is stopping...\n");
				}
			}

			
		}else{
			/*QUI SONO NEL CASO IN CUI NEL FREEFRAMELIST C'è UN FRAME LIBERO*/
			/*PT update*/
			pt_update(as->pt, as->entry_valid, frame_number, NFRAMES, pt_index);
			
			/*TLB update*/
			ret_TLB_value = tlb_insert(0, frame_number, 0,faultaddress);
			
			
			if (ret_TLB_value == 0){
				kprintf("TLB was not FULL, new TLB entry is loaded!\n");
			}else{
				kprintf("TLB was FULL, new TLB entry is loaded by REPLACEMENT ALGORITHM!\n");
			}

			
			/*Checking where the frame has to be loaded from*/
			if(load_from_elf == 1){ /*frame out from swap_file*/
				
				/*check if the faultaddress comes from DATA seg, CODE seg*/
                                switch(which_segment(as,faultaddress)){
                                        case CODE_SEG:
                                                off_fromELF = offset_fromELF(as->code_seg_start, faultaddress, (as -> code_seg_size), (as -> code_seg_offset));
					        flagRWX = 0;
                                        break;
                                        case DATA_SEG:
                                                off_fromELF = offset_fromELF(as->data_seg_start, faultaddress, (as -> data_seg_size), (as -> data_seg_offset));
					        flagRWX = 1;
                                        break;
                                        case ERR_SEG:
                                                panic("INVALID faultaddress. The program is stopping...\n");
                                        default:
                                                panic("INVALID faultaddress. The program is stopping...\n");
                                                flagRWX =0;
                                                off_fromELF =0;
                                }
				
				/*load the frame from elf_file*/	
				/*i will load the frame from elf_file*/
				ret_value = load_page_fromElf(off_fromELF, faultaddress, PAGE_SIZE, PAGE_SIZE, flagRWX);
				
				if(ret_value ==0){
					kprintf("Frame is loaded from elfFile!\n");
				}else{
					panic("ERROR load from ElfFile! the program is stopping...\n");
				}
			}else{
				/*i will load the frame from swap_file*/
				index_swapfile = search_swapped_frame(faultaddress);/*La funzione vuole come parametro l'indirizzo virtuale della pagina
											da cercare nello swapfile*/
				/*Check if the searching was successful*/
				if(index_swapfile ==0){
					panic("ERROR search_swapped_frame function was NO successful! the program is stopping...\n");
				}
                        
				ret_value = swap_pagein(get_page_number(vbase1,as->entry_valid), index_swapfile);/*La funzione "swap_pagein" vuole:
													PRIMO parametro --> l'indirizzo virtuale della pagina da portare in memoria
													SECONDO parametro --> Posizione (offset) corrispondente alla relativa pagina nello swap File*/	
				if(ret_value ==0){
					kprintf("Swap_in page from swap_file is DONE!\n");
				}else{
					panic("ERROR swap_in page from swap_file! the program is stopping...\n");
				}
			
			}

			
		}

	}else{
		/*TLB update*/
		ret_TLB_value = tlb_insert(as->pt[pt_index], 0, 1,faultaddress);
	}

        return 0;
}