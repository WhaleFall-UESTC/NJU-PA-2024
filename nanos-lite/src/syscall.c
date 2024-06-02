#include <common.h>
#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <fs.h>
#include <sys/time.h>
#include <proc.h>


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  
  switch (a[0]) {
    case SYS_exit:  
      // context_uload(current, "/bin/nterm", (char* const[]){"/bin/nterm", NULL}, (char* const[]){NULL});
      // switch_boot_pcb();
      // yield();
      // assert(0); 
      // halt(c->GPR2);
      c->GPRx = 0;
      break;
    case SYS_yield:  c->GPRx = 0; yield(); break;

    case SYS_open: {
      char *path = (char *) c->GPR2;
      int flags = c->GPR3;
      int mode = c->GPR4;

      c->GPRx = fs_open(path, flags, mode);
      // Log("sys open %s, fd = %d", path, c->GPRx);
      break;
    }

    case SYS_write: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      c->GPRx = fs_write(fd, buf, len);
      // Log("sys write to fd:%d %s, len = %d", fd, get_filename(fd), len);
      break;
    }

    case SYS_read: {
      int fd = c->GPR2;
      char *buf = (char *) c->GPR3;
      int len = c->GPR4;

      c->GPRx = fs_read(fd, buf, len);
      // Log("sys read from fd: %d %s, len = %d", fd, get_filename(fd), len);
      break;
    }

    case SYS_lseek: {
      int fd = c->GPR2;
      int offset = c->GPR3;
      int whence = c->GPR4;

      c->GPRx = fs_lseek(fd, offset, whence);
      // Log("sys lseek fd:%d %s, offset switch to %d", fd, get_filename(fd), c->GPRx);
      break;
    }

    case SYS_brk: {
      
      c->GPRx = 0; 
      break;
    }

    case SYS_gettimeofday: {
      uint64_t us = io_read(AM_TIMER_UPTIME).us;
      struct timeval *tv = (struct timeval *) c->GPR2;
      tv->tv_sec = us / 1000000;
      tv->tv_usec = us % 1000000;
      c->GPRx = 0;
    }

    case SYS_close: c->GPRx = 0; break;

    case SYS_execve: {
      char *path = (char *) c->GPR2;
      char **argv = (char **) c->GPR3;
      char **envp = (char **) c->GPR4;

      if (fs_open(path, 0, 0) == -1) {
        c->GPRx = -2;
        break;
      }

      Log("argv: %p, envp: %p", argv, envp);
      Log("SYS_execve: %s", path);
      int argc, envc;
      for (argc = 0; argv[argc]!= NULL; argc++) 
        Log("argv[%d]: %s", argc, argv[argc]);
      for (envc = 0; envp[envc]!= NULL; envc++)
        Log("envp[%d]: %s", envc, envp[envc]);
      Log("argc = %d, envc = %d", argc, envc);

      context_uload(current, path, argv, envp);

      if (current->cp == NULL) {
        panic("Warning execve failed");
      } else {
        switch_boot_pcb();
        yield();
        assert(0);
      }
      break;
    }

    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
