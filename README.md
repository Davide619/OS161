# OS161
## WILLIAM
- in addrspace.c dentro la vm_fault, il debugger si blocca su pt_update, qualche errore
  su questa funzione riporta il debugger ad eseguire la ffl_pop più volte, fino a quando
  questa non si svuota richiamando l'algoritmo di page replacement; a questo punto nella
  sezione sul page replacement, get_victim_frame manda in crash il kernel.
  
