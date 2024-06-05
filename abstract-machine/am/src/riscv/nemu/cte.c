#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

#define CONTEXT_SIZE  ((NR_REGS + 3) * XLEN)
#define XLEN  4
#define NR_REGS 32

static Context* (*user_handler)(Event, Context*) = NULL;
extern void __am_get_cur_as(Context *c);
extern void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case -1: ev.event = EVENT_YIELD; break;
      case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 10:
      case 11:case 12:case 13:case 14:case 15:case 16:case 17:case 18:case 19:
       ev.event = EVENT_SYSCALL; break;
      default: ev.event = EVENT_ERROR; break;
    }

    // for (int i = 0; i < 16; i++) {
    //   printf("%d: %d\t\t%d: %d\n", i, c->gpr[i], i + 16, c->gpr[i + 16]);
    // }
    // printf("mcause:%d\tmstatus:%d\tmepc:%d\n", c->mcause, c->mstatus, c->mepc);

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
  // 将 __am_asm_trap 函数的地址写入 mtvec

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *new_c = (Context *) kstack.end - 1;
  // printf("Loading... Stack_end: %#08x, and Context: %#08x, entry: %#08x, sizeof Context is %d\n", kstack.end, new_c, entry, sizeof(Context));
  new_c->gpr[2] = (uintptr_t) kstack.end;
  new_c->mepc = (uintptr_t) entry;
  new_c->GPR2 = (uintptr_t) arg;
  new_c->mstatus = 0x1800;
  return new_c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
