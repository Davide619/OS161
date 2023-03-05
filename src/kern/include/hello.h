#ifndef _SYSCALL_H			/*rendo visibile questa struttora solo al kernel SYSCALL*/
#define _SYSCALL_H

#include "opt-syscall.h"

#if OPT_SYSCALL
void hello(void);
#endif

#endif