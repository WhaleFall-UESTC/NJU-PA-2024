#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

/* AddrSpace defined in am-origin.h
typedef struct {
  int pgsize;
  Area area;
  void *ptr;
} AddrSpace;
*/

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;
void naive_uload(PCB *pcb, const char *filename);

#endif
