# OS161
## WILLIAM
- in "menu.c" a riga 136 (funzione "common_prog") non viene gestita 
  la fase di distruzione del processo una volta che l'esecuzione è 
  terminata, questa potrebbe essere una possibile causa di errore.
- attualmente l'esecuzione di palin non chiama più out of memory, 
  però non esegue nulla e, cosa più strana ancora, il kernel riesce 
  comunque a stampare nuovamente la schermata di attesa per il comando 
  di input.
- nella common_prog viene creato un child process per eseguire il 
  programma, però il parent non attende che l'esecuzione del programma 
  eseguito dal child termini, ma ritorna immediatamente al menu. 
  Questa in un certo senso può essere la ragione per cui non vediamo 
  alcun output da parte di palin, perché non diamo il tempo al child 
  di eseguire le operazioni ma il parent ritorna subito al menu.
- il mio debugger si ferma nella loadelf(), quando cerco di richiamare 
  as_prepare_load2().
  All'interno di as_prepare_load2() funziona tutto fino a as_zero_region() 
  (segments.c, riga 379), quando cerco di eseguirla il debugger si blocca 
  e non prosegue.
