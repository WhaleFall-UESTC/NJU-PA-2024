#include <common.h>
#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  
  switch (a[0]) {
    case SYS_exit:  c->GPRx = 0; halt(0); break;
    case SYS_yield:  c->GPRx = 0; yield(); break;

    case SYS_write: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      if (fd == 1 || fd == 2) {
        for (int i = 0; i < len; i++)
          putch(*buf++);
      }
      
      break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
