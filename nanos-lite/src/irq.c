#include "common.h"

//pa3 level1: add do_event
extern _RegSet *do_syscall(_RegSet *r);
extern _RegSet *schedule(_RegSet *prev);

static _RegSet *do_event(_Event e, _RegSet *r)
{
  switch (e.event)
  {
  case _EVENT_SYSCALL:
    do_syscall(r);
    return schedule(r);
  //pa4 level2 add _EVENR_TRAP
  case _EVENT_TRAP:
  {
    printf("event: self-trapped\n");
    //do_syscall(r);
    return schedule(r);
    break;
  }
  default:
    panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void)
{
  _asye_init(do_event);
}
