#include <common.h>
#include "syscall.h"
#include <fs.h>
#include <sys/time.h>


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  
  switch (a[0]) {
    case SYS_exit:  c->GPRx = 0; halt(0); Log("SYS_exit"); break;
    case SYS_yield: c->GPRx = 0; yield(); Log("SYS_yield");break;

    case SYS_write: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      c->GPRx = fs_write(fd, buf, len);
      Log("SYS_write fd=%d file:%s", fd, get_filename(fd));
      break;
    }

    case SYS_brk: {
      c->GPRx = 0; 
      Log("SYS_brk");
      break;
    }

    case SYS_open: {
      char *filename = (char *) c->GPR2;
      int fd = fs_open(filename);
      if (fd < 0) panic("fs_open(%s) returned -1", filename);
      c->GPRx = fd;
      Log("SYS_open fd=%d file:%s", fd, get_filename(fd));
      break;
    }

    case SYS_read: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;
      c->GPRx = fs_read(fd, buf, len);
      Log("SYS_read fd=%d file:%s", fd, get_filename(fd));
      break;
    }

    case SYS_lseek: {
      int fd = c->GPR2;
      int offset = c->GPR3;
      int whence = c->GPR4;
      c->GPRx = fs_lseek(fd, offset, whence);
      Log("SYS_lseek fd=%d file:%s old_off:%p cur_off:%p", fd, get_filename(fd), offset, c->GPRx);
      break;
    }

    case SYS_close: {
      // int fd = c->GPR2;
      c->GPRx = fs_close();
      Log("SYS_close");
      break;
    }

    case SYS_gettimeofday: {
      struct timeval *tv = (struct timeval *) c->GPR2;
      // struct timezone *tz = (struct timezone *) c->GPR3;

      uint64_t us = io_read(AM_TIMER_UPTIME).us;
      tv->tv_sec = us / 1000000;
      tv->tv_usec = us % 1000000;

      c->GPRx = 0;
      Log("SYS_gettimeofday");
      break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
