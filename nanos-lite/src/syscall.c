#include <common.h>
#include "syscall.h"
#include <stdio.h>

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: printf("Ciallo\n"); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
