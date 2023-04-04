
/*FILE .c to count all faults needed for statistics 
at the shutdown of the program                  */

#include <vmstats.h>

uint32_t cnt[10]={0};

uint32_t TLB_Faults(void){
    return cnt[0]++;
}
uint32_t TLB_Faults_with_Free(void){
    return cnt[1]++;
}
uint32_t TLB_Faults_with_Replace(void){
    return cnt[2]++;
}
uint32_t TLB_Invalidations(void){
    return cnt[3]++;
}
uint32_t TLB_Reloads(void){
    return cnt[4]++;
}
uint32_t PageFaults_Zeroed(uint32_t count){
    cnt[5] = count;
    return cnt[5];
}
uint32_t PageFaults_Disk(void){
    return cnt[6]++;
}
uint32_t PageFaults_from_ELF(void){
    return cnt[7]++;
}
uint32_t PageFaults_from_Swapfile(void){
    return cnt[8]++;
}
uint32_t SwapfileWrites(void){
    return cnt[9]++;
}

void print_all_stats(void)
{
    
    kprintf("Statistics : \n");
    kprintf("TLB_Faults : %d\n",cnt[0]+NFRAMES);
    kprintf("TLB_Faults_with_Free : %d\n",cnt[1]+NFRAMES);
    kprintf("TLB_Faults_with_Replace : %d\n",cnt[2]);
    kprintf("TLB_Invalidations : %d\n",cnt[3]);
    kprintf("TLB_Reloads : %d\n",cnt[4]);
    kprintf("PageFaults_Zeroed : %d\n",cnt[5]);
    kprintf("PageFaults_Disk : %d\n",cnt[6]);
    kprintf("PageFaults_from_ELF : %d\n",cnt[7]);
    kprintf("PageFaults_from_Swapfile : %d\n",cnt[8]);
    kprintf("SwapfileWrites : %d\n",cnt[9]);

}