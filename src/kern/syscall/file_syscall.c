/*
 * AUthor: G.Cabodi
 * Very simple implementation of sys__exit.
 * It just avoids crash/panic. Full process exit still TODO
 * Address space is released
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <mips/trapframe.h>
#include <current.h>
#include <synch.h>
#include <kern/wait.h>

/*
 * system calls for process management
 */
int 
sys_read(int fd, userptr_t buf_ptr, size_t size) {
  int i;
  char *p = (char *)buf_ptr;

  if (fd!=STDIN_FILENO) {
    kprintf("sys_read supported only to stdin\n");
    return -1;
  }

  for (i=0; i<(int)size; i++) {
    p[i] = getch();
    if (p[i] < 0) 
      return i;
  }

  return (int)size;
}


int 
sys_write(int fd, userptr_t buf_ptr, int size) {
  int i;
  char *p = (char *)buf_ptr;

  if (fd!=STDOUT_FILENO && fd!=STDERR_FILENO) {
    kprintf("sys_write supported only to stdout\n");
    return -1;
  }

  for (i=0; i<(int)size; i++) {
    putch(p[i]);
  }

  return (int)size;
}


void sys__exit(int status){   /*(int status)*/
   struct addrspace *as = proc_getas();

   as_destroy(as);
 
   thread_exit();
   
   /* thread_exit() does not return, so we should never get here */
   panic("return from thread_exit in sys_exit\n");
   (void)status;
  
}


// int
// sys_waitpid(pid_t pid, userptr_t statusp, int options)
// {
// #if OPT_WAITPID
//   struct proc *p = proc_search_pid(pid);
//   int s;
//   (void)options; /* not handled */
//   if (p==NULL) return -1;
//   s = proc_wait(p);
//   if (statusp!=NULL) 
//     *(int*)statusp = s;
//   return pid;
// #else
//   (void)options; /* not handled */
//   (void)pid;
//   (void)statusp;
//   return -1;
// #endif
// }

// pid_t
// sys_getpid(void)
// {
// #if OPT_WAITPID
//   KASSERT(curproc != NULL);
//   return curproc->p_pid;
// #else
//   return -1;
// #endif
// }

/* #if OPT_FORK
static void
call_enter_forked_process(void *tfv, unsigned long dummy) {
  struct trapframe *tf = (struct trapframe *)tfv;
  (void)dummy;
  enter_forked_process(tf); 
 
  panic("enter_forked_process returned (should not happen)\n");
}

int sys_fork(struct trapframe *ctf, pid_t *retval) {
  struct trapframe *tf_child;
  struct proc *newp;
  int result;

  KASSERT(curproc != NULL);

  newp = proc_create_runprogram(curproc->p_name);
  if (newp == NULL) {
    return ENOMEM;
  }
 */
  /* done here as we need to duplicate the address space 
     of thbe current process */
 /*  as_copy(curproc->p_addrspace, &(newp->p_addrspace));
  if(newp->p_addrspace == NULL){
    proc_destroy(newp); 
    return ENOMEM; 
  } */

  /* we need a copy of the parent's trapframe */
/*   tf_child = kmalloc(sizeof(struct trapframe));
  if(tf_child == NULL){
    proc_destroy(newp);
    return ENOMEM; 
  }
  memcpy(tf_child, ctf, sizeof(struct trapframe));
 */
  /* TO BE DONE: linking parent/child, so that child terminated 
     on parent exit */

/*   result = thread_fork(
		 curthread->t_name, newp,
		 call_enter_forked_process, 
		 (void *)tf_child, (unsigned long)0*//*unused*//*);
*/

/*
  if (result){
    proc_destroy(newp);
    kfree(tf_child);
    return ENOMEM;
  }

  *retval = newp->p_pid;

  return 0;
}
#endif
 */
