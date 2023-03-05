#include <segments.h>

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vnode.h>
#include <elf.h>
#include <vfs.h>

#include <kern/fcntl.h>
#include <mips/tlb.h>

static struct vnode *vn;

int which_segment(struct addrspace *as, vaddr_t faultaddress){
    /*In questi if controllo se l'informazione che ricevo dall'indirizzo virtuale fa parte del code, data o stack*/
    if ((faultaddress >= (as->code_seg_start)) && (faultaddress < ((as->code_seg_size -1)+(as->code_seg_start))))
    {       
        /*code*/
		return CODE_SEG;
    }
    else if ((faultaddress >= (as->data_seg_start)) && (faultaddress < ((as->data_seg_size -1)+(as->data_seg_start))))
    {
        /*data*/
	    return DATA_SEG;
    }
        			
    else  return ERR_SEG; /*Ritorno un errore qualora tale indirizzo non fa parte di nessuno di questi casi*/
}



/*OffsetFromELF function*/
off_t
offset_fromELF(vaddr_t segment_start, vaddr_t faultaddress, size_t segment_size, off_t segment_offset){

        off_t ret_offset;
        /* Compute the segment relative page */
        unsigned int npage_from_start = ((faultaddress & PAGE_FRAME) - (segment_start & PAGE_FRAME))/PAGE_SIZE;
        
        
        if(npage_from_start == 0){
                /* in case we are in the first page of the segment*/
                ret_offset = segment_offset;
                
        }else if(((faultaddress - segment_start)&PAGE_FRAME) > segment_size){
                /* in case we are beyond the size of segment. This means that i will have zeros in that part of elf-File*/
                ret_offset = segment_offset + segment_size;
                
        }else{
                /* intermidiate page */
                ret_offset = segment_offset + (PAGE_SIZE - segment_offset % PAGE_SIZE) + (npage_from_start-1)*PAGE_SIZE;
                
        }
        
        return ret_offset;

}



int
load_page_fromElf(off_t offset, vaddr_t vaddr,
             size_t memsize, size_t filesize,
             int is_executable)
{
        struct addrspace *as;
        struct iovec iov;
        struct uio u;
        int result;

        char *progname = proc_getprogname();

        as = proc_getas();


        /* Open the file. */
        result = vfs_open(progname, O_RDONLY, 0, &vn);
        if (result) {
                return result;
        }

        if (filesize > memsize) {
                kprintf("ELF: warning: segment filesize > segment memsize\n");
                filesize = memsize;
        }

        DEBUG(DB_EXEC, "ELF: Loading %lu bytes to 0x%lx\n",
              (unsigned long) filesize, (unsigned long) vaddr);
                                                                               
        iov.iov_ubase = (userptr_t)vaddr;
        iov.iov_len = PAGE_SIZE;           // length of the memory space
        u.uio_iov = &iov;
        u.uio_iovcnt = 1;
        u.uio_resid = PAGE_SIZE;          // amount to read from the file
        u.uio_offset = offset;
        u.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE;
        u.uio_rw = UIO_READ;
        u.uio_space = as;

        result = VOP_READ(vn, &u);               
        if (result) {

                vfs_close(vn);
                return result;
        }
        if (u.uio_resid != 0) {
                /* short read; problem with executable? */
                vfs_close(vn);
                kprintf("ELF: short read on segment - file truncated?\n");
                return ENOEXEC;
        }

        vfs_close(vn);

        /*
         * If memsize > filesize, the remaining space should be
         * zero-filled. There is no need to do this explicitly,
         * because the VM system should provide pages that do not
         * contain other processes' data, i.e., are already zeroed.
         *
         * During development of your VM system, it may have bugs that
         * cause it to (maybe only sometimes) not provide zero-filled
         * pages, which can cause user programs to fail in strange
         * ways. Explicitly zeroing program BSS may help identify such
         * bugs, so the following disabled code is provided as a
         * diagnostic tool. Note that it must be disabled again before
         * you submit your code for grading.
         */
#if 0
        {
                size_t fillamt;

                fillamt = memsize - filesize;
                if (fillamt > 0) {
                        DEBUG(DB_EXEC, "ELF: Zero-filling %lu more bytes\n",
                              (unsigned long) fillamt);
                        u.uio_resid += fillamt;
                        result = uiomovezeros(fillamt, &u);
                }
        }
#endif

        return result;
}



/*
 * Load an ELF executable user program into the current address space.
 *
 * Returns the entry point (initial PC) for the program in ENTRYPOINT.
 */
int
load_elf(struct vnode *v, vaddr_t *entrypoint)
{
        Elf_Ehdr eh;   /* Executable header */
        Elf_Phdr ph;   /* "Program header" = segment header */
        int result, i, ras;
        struct iovec iov;
        struct uio ku;
        struct addrspace *as;

        as = proc_getas();                                     

        /*
         * Read the executable header from offset 0 in the file.
         */

        uio_kinit(&iov, &ku, &eh, sizeof(eh), 0, UIO_READ);
        result = VOP_READ(v, &ku);                                      
        if (result) {
                return result;
        }

        if (ku.uio_resid != 0) {
                /* short read; problem with executable? */
                kprintf("ELF: short read on header - file truncated?\n");
                return ENOEXEC;
        }

        /*
         * Check to make sure it's a 32-bit ELF-version-1 executable
         * for our processor type. If it's not, we can't run it.
         *
         * Ignore EI_OSABI and EI_ABIVERSION - properly, we should
         * define our own, but that would require tinkering with the
        */

        if (eh.e_ident[EI_MAG0] != ELFMAG0 ||
            eh.e_ident[EI_MAG1] != ELFMAG1 ||
            eh.e_ident[EI_MAG2] != ELFMAG2 ||
            eh.e_ident[EI_MAG3] != ELFMAG3 ||
            eh.e_ident[EI_CLASS] != ELFCLASS32 ||
            eh.e_ident[EI_DATA] != ELFDATA2MSB ||
            eh.e_ident[EI_VERSION] != EV_CURRENT ||
            eh.e_version != EV_CURRENT ||
            eh.e_type!=ET_EXEC ||
            eh.e_machine!=EM_MACHINE) {
                return ENOEXEC;
        }

        /*
         * Go through the list of segments and set up the address space.
         *
         * Ordinarily there will be one code segment, one read-only
         * data segment, and one data/bss segment, but there might
         * conceivably be more. You don't need to support such files
         * if it's unduly awkward to do so.
         *
         * Note that the expression eh.e_phoff + i*eh.e_phentsize is
         * mandated by the ELF standard - we use sizeof(ph) to load,
         * because that's the structure we know, but the file on disk
         * might have a larger structure, so we must use e_phentsize
         * to find where the phdr starts.
         */

        for (i=0; i<eh.e_phnum; i++) {          

                off_t offset = eh.e_phoff + i*eh.e_phentsize;
                uio_kinit(&iov, &ku, &ph, sizeof(ph), offset, UIO_READ);

                result = VOP_READ(v, &ku);                                                     
                if (result) {
                        return result;
                }

                if (ku.uio_resid != 0) {
                        /* short read; problem with executable? */
                        kprintf("ELF: short read on phdr - file truncated?\n");
                        return ENOEXEC;
                }

                switch (ph.p_type) {
                    case PT_NULL: /* skip */ continue;
                    case PT_PHDR: /* skip */ continue;
                    case PT_MIPS_REGINFO: /* skip */ continue;
                    case PT_LOAD: 
                    break;
                    default:
                        kprintf("loadelf: unknown segment type %d\n",
                                ph.p_type);
                        return ENOEXEC;
                }
                /*definisco la regione di memoria passando anche le info dell'elf-file*/
                result = as_define_region(as,
                        ph.p_vaddr, ph.p_memsz,ph.p_offset,ph.p_filesz,
                        ph.p_flags & PF_R,
                        ph.p_flags & PF_W,
                        ph.p_flags & PF_X);
                if (result) {
                        return result;
                }

        }

        /*assegno al processo corrente as*/
        //proc_setas(as); /*forse non è necessario perchè sono già nel processo as*/
        ras = as_prepare_load2(as);
        
        if(ras ==0)
                kprintf("STRUCTURES MEMORY ALLOCATEDD! \n");

        result = as_complete_load(as);                                       
        if (result) {
                return result;
        }
        

        vn = v;

        *entrypoint = eh.e_entry;  

        return 0;
}
