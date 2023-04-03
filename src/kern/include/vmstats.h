

uint32_t TLB_Faults(void);

uint32_t TLB_Faults_with_Free(void);

uint32_t TLB_Faults_with_Replace(void);

uint32_t TLB_Invalidations(void);

uint32_t TLB_Reloads(void);

uint32_t PageFaults_Zeroed(uint32_t cnt);

uint32_t PageFaults_Disk(void);

uint32_t PageFaults_from_ELF(void);

uint32_t PageFaults_from_Swapfile(void);

uint32_t SwapfileWrites(void);

void print_all_stats(void);