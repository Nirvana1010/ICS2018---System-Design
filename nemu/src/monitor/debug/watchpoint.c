#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].value_old = 0;
    wp_pool[i].value_new = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

bool init = false;

WP* new_wp() {
  WP *temp;  

  if(init == false)
  {
    init_wp_pool();
    init = true;
  }

  if(free_ != NULL)
  {
    temp = free_;
    free_ = free_->next;
    temp->next = head;
    head = temp;
    return head;
  }
  else
    assert(0);
}

void free_wp(WP *wp) {
  WP *temp;
  WP *curr;
  bool succ = false;

  if(head == NULL)
    assert(0);
  if(head->NO == wp->NO)
  {
    succ = true;
    temp = head;
    head = head->next;
    temp->next = free_;
    free_ = temp;
    printf("delete watchpoint %d\n", temp->NO);
    return ;
  }
  
  curr = head;
  while((curr->next != NULL) && (curr->next->next != NULL))
  {
    if(curr->next->NO == wp->NO)
    {
      succ = true;
      temp = curr->next;
      curr->next = temp->next;
      temp->next = free_;
      free_ = temp;
      printf("delete watchpoint %d\n", temp->NO);
      break;
    }
    curr = curr->next;
  }
  if((curr->next != NULL) && (curr->next->NO == wp->NO) && (curr->next->next == NULL))
  {
    succ = true;
    temp = curr->next;
    curr->next = NULL;
    temp->next = free_;
    free_ = temp;
    printf("delete watchpoint %d\n", temp->NO);
  }
  
  if(succ == false)
    printf("no watchpoint to delete\n");

  return ;
}

void set_watchpoint(char *args) {
  bool succ = true; 
  WP *wp = new_wp();
  strcpy(wp->expr, args);
  
  wp->value_old = expr(wp->expr, &succ);
  if(succ == false)
  {
    printf("watchpoint expression error\n");
    free_wp(wp);
    assert(0);
  }

  printf("set watchpoint %d\n", wp->NO);
  printf("value is %#x\n", wp->value_old);
  return ;
}

void check_watchpoint(int diff[]) {
  int i = 0;
  WP *curr = head;
  bool succ = true;

  for(i = 0; i < 32; i++)
    diff[i] = -1;
  i = 0;

  while(curr != NULL)
  {
    curr->value_new = expr(curr->expr, &succ);
    if(curr->value_new != curr->value_old)
    {
      diff[i] = curr->NO;
      curr->value_old = curr->value_new;
      i++;
    }
    curr = curr->next;
  }
  return ;
}

void print_watchpoint() {
  WP *curr = head;
  if(head == NULL)
  {
    printf("No watchpoint to print!\n");
    return ;
  }
  printf("NO.\texpr\tvalue\n");
  while(curr != NULL)
  {
    printf("%d\t%s\t%#x\n", curr->NO, curr->expr, curr->value_old);
    curr = curr->next;
  }
  return ;
}
