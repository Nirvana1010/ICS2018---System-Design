#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  char expr[64];
  int value_old;
  int value_new;
  
  struct watchpoint *next;

  /* TODO: Add more members if necessary */


} WP;

extern void free_wp(WP *wp);

extern void set_watchpoint(char *args);

extern void check_watchpoint(int diff[]);

extern void print_watchpoint();
#endif
