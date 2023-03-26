#ifndef _SEGMENTS_H_
#define _SEGMENTS_H_

#include <addrspace.h>
#include <vnode.h>
#include <types.h>


/*Macros*/
/*return values of "which_segment" function*/
#define DATA_SEG 0 
#define CODE_SEG 1
#define STACK_SEG 2
#define ERR_SEG 3

/*function that checks from which segment faultaddress comes*/
int which_segment(struct addrspace *as, vaddr_t faultaddress);

/*Computes the offset from elf file*/
//off_t offset_fromELF(vaddr_t segment_start, vaddr_t faultaddress, size_t segment_size, off_t segment_offset);
int offset_fromELF(vaddr_t segment_start, vaddr_t faultaddress, size_t segment_size, off_t segment_offset);

/*load page function*/
int load_page_fromElf(int offset, vaddr_t vaddr, size_t memsize, size_t filesize, int is_executable);

/*load elf function*/
int load_elf(struct vnode *v, vaddr_t *entrypoint);

#endif /* _SEGMENTS_H_ */