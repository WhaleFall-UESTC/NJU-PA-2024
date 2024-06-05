#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  Log("switch boot pcb");
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // if (j % 1000000000 == 0) 
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  Log("Initializing processes...");
  context_uload(&pcb[0], "/bin/hello", (char *const[]){"/bin/hello", NULL}, (char *const[]){NULL});
  // context_uload(&pcb[1], "/bin/exec-test", (char *const[]){"/bin/exec-test", "114514", NULL}, (char *const[]){NULL});
  // context_uload(&pcb[1], "/bin/hello", (char *const[]){"/bin/hello", NULL}, (char *const[]){NULL});
  switch_boot_pcb();


  // load program here
  // naive_uload(NULL, "/bin/menu"); 
}

Context* schedule(Context *prev) {
  // Log("Switch process");
  current->cp = prev;
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}


