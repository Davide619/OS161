/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
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
#include <lib.h>
#include <bitmap.h>
#include <vm.h>
#include <mainbus.h>


vaddr_t firstfree;   /* first free virtual address; set by start.S */

static paddr_t firstpaddr;  /* address of first free physical page */
static paddr_t lastpaddr;   /* one past end of last free physical page */

static struct bitmap * frame_alloc; /* bitmap tracking allocated RAM frames */
static uint8_t * alloc_size; /* array related to bitmap, storing number of contiguous allocated frames */
static paddr_t start_frame; /* reference frame address */
static size_t nframes; /* amount of available RAM frames */

static bool ram_initialized = false; /* to start doing RAM management after ram_bootstrap completed */

/*
 * Called very early in system boot to figure out how much physical
 * RAM is available.
 */
void
ram_bootstrap(void)
{
	size_t ramsize;

	/* Get size of RAM. */
	ramsize = mainbus_ramsize();

	/*
	 * This is the same as the last physical address, as long as
	 * we have less than 512 megabytes of memory. If we had more,
	 * we wouldn't be able to access it all through kseg0 and
	 * everything would get a lot more complicated. This is not a
	 * case we are going to worry about.
	 */
	if (ramsize > 512*1024*1024) {
		ramsize = 512*1024*1024;
	}

	lastpaddr = ramsize;

	/*
	 * Get first free virtual address from where start.S saved it.
	 * Convert to physical address.
	 */
	firstpaddr = firstfree - MIPS_KSEG0; 

	kprintf("%uk physical memory available\n",
		(lastpaddr-firstpaddr)/1024); /* prints amount of memory in kB format */

	KASSERT((firstpaddr % PAGE_SIZE) == 0); /* check if it's really a PAGE_SIZE multiple */

	/* Allocate the proper space for frame_alloc & alloc_size...
	 */
	nframes = (lastpaddr - firstpaddr) / PAGE_SIZE; // C5 (hex), 197 (dec)
	frame_alloc = bitmap_create(nframes); // 25 bytes + 2 bytes for struct allocation
	alloc_size = kmalloc(nframes); // 197 bytes
	// 197 + 27 = 224 bytes allocated: just one page needed to handle frame_alloc & alloc_size

	/* ...then initialize alloc_size...
	 */
	bzero((void *) alloc_size, nframes);
	start_frame = firstpaddr;

	/* ...and mark the first page as allocated: we reserved space
	 *	  for frame_alloc & alloc_size structures
	 */
	bitmap_mark(frame_alloc, 0);
	alloc_size[0] = 1;

	/* finally, set ram_initialized: the RAM management system is ready */
	ram_initialized = true;
}

/*
 * This function is for allocating physical memory prior to VM
 * initialization.
 *
 * The pages it hands back will not be reported to the VM system when
 * the VM system calls ram_getsize(). If it's desired to free up these
 * pages later on after bootup is complete, some mechanism for adding
 * them to the VM system's page management must be implemented.
 * Alternatively, one can do enough VM initialization early so that
 * this function is never needed.
 *
 * Note: while the error return value of 0 is a legal physical address,
 * it's not a legal *allocatable* physical address, because it's the
 * page with the exception handlers on it.
 *
 * This function should not be called once the VM system is initialized,
 * so it is not synchronized.
 */
paddr_t
ram_stealmem(unsigned long npages)
{
	size_t size;
	paddr_t paddr;

	size = npages * PAGE_SIZE;

	if (firstpaddr + size > lastpaddr) {
		return 0;
	}

	/* Allocation before ram_bootstrap start to execute 
	 */
	if(!ram_initialized) {
		paddr = firstpaddr;
		firstpaddr += size;
	}

	/* Apply RAM management only if the RAM management system is initialized, 
	 * that is only if ram_bootstrap finished execution
	 */ 
	else {
		uint8_t alloc_idx = 1; /* index to traverse frame_alloc and find a free region to reserve */
		uint8_t free_frames = 0; /* counts the number of contiguous free frames*/

		/* loop to traverse all the bitmap structure, it's necessary to find
		 * a contiguous region of frames wide enough to be allocated: a free
		 * frame is unmarked, so it's sufficient to find a group of contiguous
		 * frames that are all unmarked; this group should contain at least 
		 * "npages" frames to be allocated, otherwise it isn't wide enough
		 */
		for( ; alloc_idx < nframes; ++alloc_idx) {

			/* check that the current frame is free... */
			if(!bitmap_isset(frame_alloc, alloc_idx)) {

				/* ...increment the free frames counter... */
				free_frames++;

				/* ...and finally check if the contiguous free frames
				 *    are equal to the requested pages to be allocated:
				 *    in this case the region to be reserved has been found
				 */
				if(free_frames == npages) {
					alloc_idx -= free_frames - 1;
					break;
				}
			}
			/* ...otherwise reset the count for free frames and go on */
			else
			{
				free_frames = 0;
			}
		}

		KASSERT(alloc_idx < nframes);



		/* finally, reserve the frames and update the allocated size
		 * related to the requested region...
		 */
		for (uint8_t i = 0; i < npages; ++i)
			bitmap_mark(frame_alloc, alloc_idx + i);
		
		alloc_size[alloc_idx] = npages;

		/* ...and evaluate the physical address to be returned */
		paddr = start_frame + alloc_idx * PAGE_SIZE;
	}

	return paddr;
}

/*
 * This function is for deallocating memory pages upon request, after 
 * calling this function the reserved frames are freed
 */
int
ram_freemem(paddr_t paddr) 
{
	uint8_t free_idx, freed_frames;

	/* find the index entry related to paddr, to access the bitmap 
	 * and reset that entry
	 */
	free_idx = (paddr - start_frame) / PAGE_SIZE;
	freed_frames = alloc_size[free_idx];
	alloc_size[free_idx] = 0;

	for (uint8_t i = 0; i < freed_frames; ++i) 
		bitmap_unmark(frame_alloc, free_idx+i);

	return freed_frames;
}

/*
 * This function is intended to be called by the VM system when it
 * initializes in order to find out what memory it has available to
 * manage. Physical memory begins at physical address 0 and ends with
 * the address returned by this function. We assume that physical
 * memory is contiguous. This is not universally true, but is true on
 * the MIPS platforms we intend to run on.
 *
 * lastpaddr is constant once set by ram_bootstrap(), so this function
 * need not be synchronized.
 *
 * It is recommended, however, that this function be used only to
 * initialize the VM system, after which the VM system should take
 * charge of knowing what memory exists.
 */
paddr_t
ram_getsize(void) 
{
	return lastpaddr;
}

/*
 * This function is intended to be called by the VM system when it
 * initializes in order to find out what memory it has available to
 * manage.
 *
 * It can only be called once, and once called ram_stealmem() will
 * no longer work, as that would invalidate the result it returned
 * and lead to multiple things using the same memory.
 *
 * This function should not be called once the VM system is initialized,
 * so it is not synchronized.
 */
paddr_t
ram_getfirstfree(void)
{
	paddr_t ret;
	
	ret = firstpaddr;
	firstpaddr = lastpaddr = 0;
	return ret;
}