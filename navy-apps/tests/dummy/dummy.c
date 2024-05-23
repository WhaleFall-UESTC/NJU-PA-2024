#include <stdint.h>
// #include <stdio.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main()
{
  // puts("This is dummy");
  // return _syscall_(SYS_yield, 1, 1, 1);
  asm volatile("li a7, 1; ecall");
  return 0;
}
