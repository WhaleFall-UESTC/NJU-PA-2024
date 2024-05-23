#include <common.h>
#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  printf("Syscall ID = %02d\t, Arguments: %2d, %2d, %2d\tRet: ", a[0], a[1], a[2], a[3]);

  switch (a[0]) {
    case SYS_exit:  printf("0\n"); c->GPRx = 0; halt(0); break;
    case SYS_yield:  printf("0\n"); c->GPRx = 0; yield(); break;

    case SYS_write: {
      if (a[1] == 1 || a[1] == 2) {
        for (int i = 0; i < a[3]; i++) printf("%c", *((char *)(a[2] + i)));
      } else {
        
      }
      printf("%d\n", a[3]); c->GPRx = a[3]; break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
