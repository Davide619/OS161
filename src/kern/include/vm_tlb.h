
#ifndef _VM_TLB
#define _VM_TLB

#include <addrspace.h>
#include <types.h>
#include <machine/vm.h>
#include <vm.h>

/*function to add a new entry. RR algorithm is included*/
int tlb_insert(paddr_t paddr1, paddr_t paddr2, int flag, vaddr_t faultaddress);

/*Round robin algorithm for victim entry*/
int tlb_get_rr_victim(void);

/*Invalidation entries function*/
int TLB_Invalidate(paddr_t paddr);

/*TLB update function*/
int TLB_update(paddr_t paddr, vaddr_t faultaddress);

/* TLB shootdown handling called from interprocessor_interrupt */
void vm_tlbshootdown(const struct tlbshootdown *);

#endif