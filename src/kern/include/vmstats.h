
#ifndef _VMSTATS_H_
#define _VMSTATS_H_

#include <types.h>
#include <lib.h>
#include <addrspace.h>

uint32_t TLB_Faults(void);

uint32_t TLB_Faults_with_Free(void);

uint32_t TLB_Faults_with_Replace(void);

uint32_t TLB_Invalidations(void);

uint32_t TLB_Reloads(void);

uint32_t PageFaults_Zeroed(uint32_t count);

uint32_t PageFaults_Disk(void);

uint32_t PageFaults_from_ELF(void);

uint32_t PageFaults_from_Swapfile(void);

uint32_t SwapfileWrites(void);

void print_all_stats(void);

#endif /* _VMSTATS_H_ */