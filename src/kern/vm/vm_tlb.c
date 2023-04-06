#include <mips/tlb.h>
#include <mips/vm.h> 
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
//#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <spl.h>
//#include <machine/spl.h>
//#include <machine/tlb.h>
#include <clock.h>

#include <vm_tlb.h>

#include <vmstats.h>

/*FUNZIONE CHE AGGIUNGE UNA NEW ENTRY ALLA TLB*/
int tlb_insert(paddr_t paddr1, paddr_t paddr2, int flag, vaddr_t faultaddress){
    int spl;
    uint32_t i, elo, ehi, victim;
    uint32_t dbflags = 0;
    paddr_t paddr;

    spl = splhigh();


    /*get the 20MSBs of the faultaddress*/
    faultaddress &= PAGE_FRAME;
    
// Look for an invalid entry on the TLB
    for (i=0; i<NUM_TLB; i++) {
        
        tlb_read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            continue;
        }
        ehi = faultaddress;
        paddr = flag ? paddr1 : paddr2;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;                //vedere in questo caso come settare il dirty bit
        DEBUG(DB_VM, "TLB Added: 0x%x -> 0x%x at location %d\n", faultaddress, paddr, i);
        tlb_write(ehi, elo, i);
        splx(spl); // Leave that to calling function
        TLB_Faults_with_Free(); //TLB fault for which replcament was not needed
        return 0;
    }

//if i am here, i have no TLB space, so i use replace algorithm

    ehi = faultaddress;
    paddr = flag ? paddr1 : paddr2;
    elo = paddr | TLBLO_DIRTY | TLBLO_VALID; 

    victim = tlb_get_rr_victim();
    DEBUG(DB_VM, "TLB Replacement: 0x%x -> 0x%x at location %d\n", faultaddress, paddr, victim);
    tlb_write(ehi, elo, victim);
    splx(spl);
    TLB_Faults_with_Replace(); //TLB fault for which replacement is needed
    return 1; //returns 1 if replace algorithm is executed 
}



/*FIFO TLB Algorithm*/
uint32_t tlb_get_rr_victim(void){
	
	uint32_t victim;
	static uint32_t next_victim;
    victim = next_victim;
	next_victim = (next_victim + 1) % NUM_TLB;
	return victim;

}



/*invalidate entry function*/
int TLB_Invalidate(paddr_t paddr)
{
    uint32_t ehi,elo,i;

    for (i=0; i<NUM_TLB; i++) 
    {
        tlb_read(&ehi, &elo, i);
        if ((elo & TLBLO_PPAGE) == (paddr & TLBLO_PPAGE)) 
        {
            tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i); 
            TLB_Invalidations(); //Here we count all the invalidations of TLB      
        }
    }

    return 0;
}


/*TLB update*/
int TLB_update(paddr_t paddr, vaddr_t faultaddress)
{
    uint32_t ehi, elo, elo_new, i;
    int spl;

    spl = splhigh(); 

    for (i=0; i<NUM_TLB; i++) 
    {
        tlb_read(&ehi, &elo, i);
        if ((ehi & TLBHI_VPAGE) == (faultaddress & TLBHI_VPAGE)) 
        {
            elo_new = paddr | TLBLO_DIRTY | TLBLO_VALID;
            tlb_write(ehi, elo_new, i);        
        }
    }
    splx(spl);

    return 0;
}

void vm_tlbshootdown(const struct tlbshootdown *ts)
{	
	(void)ts;
	panic("DB-MM VM tried to do tlb shootdown?!\n");
}