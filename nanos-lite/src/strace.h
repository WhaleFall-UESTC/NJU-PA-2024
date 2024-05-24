#include <stdio.h>

const char* strace_path = "strace.log";
FILE *strace = NULL;

void strace_init();
void strace_log(int no, const char * type, uintptr_t* argum);