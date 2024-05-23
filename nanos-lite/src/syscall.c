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

  switch (a[0]) {
    case SYS_exit:  printf("SYS_exit\n"); c->GPRx = 0; halt(0); break;
    case SYS_yield:  printf("SYS_yield\n"); c->GPRx = 0; yield(); break;

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
