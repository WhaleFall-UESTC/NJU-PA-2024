#include <common.h>
#include "syscall.h"
#include <fs.h>


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  
  switch (a[0]) {
    case SYS_exit:  c->GPRx = 0; halt(0); break;
    case SYS_yield: c->GPRx = 0; yield(); break;

    case SYS_write: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      

      // if (fd == 1 || fd == 2) {
      //   for (int i = 0; i < len; i++)
      //     putch(*buf++);
      // } else {
      //   c->GPRx = -1;
      //   break;
      // }

      c->GPRx = fs_write(fd, buf, len);;
      break;
    }

    case SYS_brk: {
      c->GPRx = 0; 
      break;
    }

    case SYS_open: {
      char *filename = (char *) c->GPR2;
      int fd = fs_open(filename);
      if (fd < 0) panic("fs_open(%s) returned -1", filename);
      c->GPRx = fd;
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

    case SYS_close: {
      // int fd = c->GPR2;
      c->GPRx = fs_close();
      break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
