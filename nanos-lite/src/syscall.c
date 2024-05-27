#include <common.h>
#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <fs.h>
#include <sys/time.h>


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  
  switch (a[0]) {
    case SYS_exit:  c->GPRx = 0; halt(0); break;
    case SYS_yield:  c->GPRx = 0; yield(); break;

    case SYS_open: {
      char *path = (char *) c->GPR2;
      int flags = c->GPR3;
      int mode = c->GPR4;

      c->GPRx = fs_open(path, flags, mode);
      break;
    }

    case SYS_write: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      c->GPRx = fs_write(fd, buf, len);
      break;
    }

    case SYS_read: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      c->GPRx = fs_read(fd, buf, len);
      break;
    }

    case SYS_lseek: {
      int fd = c->GPR2;
      int offset = c->GPR3;
      int whence = c->GPR4;

      c->GPRx = fs_lseek(fd, offset, whence);
      break;
    }

    case SYS_brk: c->GPRx = 0; break;

    case SYS_gettimeofday: {
      uint64_t us = io_read(AM_TIMER_UPTIME).us;
      struct timeval *tv = (struct timeval *) c->GPR2;
      tv->tv_sec = us / 1000000;
      tv->tv_usec = us % 1000000;
      c->GPRx = 0;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
