#include <common.h>

void do_syscall(Context *c);
Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case 1: Log("Detect EVENT_YIELD"); c = schedule(c); break;
    case 2: /*Log("Detect EVENT_SYSCALL");*/ do_syscall(c); break;
    case 5: /*Log("Detect EVENT_IRQ_TIMER");*/ c = schedule(c); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
