#include "strace.h"
#include <stdio.h>
#include <stdlib.h>

void strace_init() {
  strace = fopen(strace_path, "w");
  fprintf(strace, "start etrace\n");
  fflush(strace);
}

void strace_log(int no, const char * type, uintptr_t* argum) {
  fprintf(strace, "NO.%2d %s\tArgument: ", no, type);
  for (int i = 0; i < sizeof(argum) / sizeof(int); i++) {
    fprintf(strace, "%#08x ", argum[i]);
  }
  fprintf(strace, "\n");
  fflush(strace);
}