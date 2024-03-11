#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  bool is_used;
  int value;
  char *expr;
} WP;

bool free_wp(int No);
WP* new_wp();
void print_wp();

bool check_wp();
#endif
