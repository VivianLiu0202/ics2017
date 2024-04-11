#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename)
{
  int i = nr_proc++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
  //pa4 level2: delete code
  // _switch(&pcb[i].as);
  // current = &pcb[i];
  // ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

//pa4 level2 : add schedule functiin
int count = 0;
//pa4 level5 add:
extern int current_game;
_RegSet *schedule(_RegSet *prev)
{
  // save the context pointer
  current->tf = prev;

  //count++;
  //Log("count = %d", count);
  //current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);

  if (count == 1000)
  {
    count = 0;
    current = &pcb[1];
  }
  else
  {
    if (current_game)
    {
      current = &pcb[0];
      count++;
    }
    else
    {
      current = &pcb[2];
      count++;
    }
  }

  Log("count = %d current_game = %d", count, current_game);
  // TODO: switch to the new address space
  // then return the new context
  _switch(&current->as);
  return current->tf;
}
