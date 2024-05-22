#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD: printf("Detect EVENT_YIELD\n"); break;
    // case 4: {
    //   printf("Detect EVENT_SYSCALL\n"); 
    //   // do_syscall(c);
    //   break;
    // }
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
