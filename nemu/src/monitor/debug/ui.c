#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  //int i;

  if(arg == NULL) {
    cpu_exec(1);
  }
  else {
    //printf("[N]:%d\n", atoi(arg));
    int n = atoi(arg);
    if(n == 0)
      printf("invalid input, please input a integer\n");
    else {
      cpu_exec(n);
    }
  }
  return 0;
}

static int cmd_help(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_d(char *args);

static int cmd_w(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "argu [N]: Execute [N] instructions step by step", cmd_si },
  { "info", "args:r/w; print information about registers or watchpoint", cmd_info },
  { "x", "x [N] [EXPR]; scan the memory", cmd_x },
  { "p", "expr", cmd_p },
  { "d", "d [N]; delete watchpoint", cmd_d },
  { "w", "w [EXPR]; set watchpoint", cmd_w },

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    //printf("args:%s\n", args);
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  int i;
  //printf("arg: %s\n", arg);
  if(arg == NULL)
    printf("Invalid input\n");
  else if(strcmp(arg, "r") == 0) {
    //printf("#################\n");
    for(i = 0; i < 8; i++)
      printf("%s  0x%x\n", regsl[i], reg_l(i));
    printf("%s  0x%x\n", "eip", cpu.eip);
    for(i = 0; i < 8; i++)
      printf("%s  0x%x\n", regsw[i], reg_w(i));
    for(i = 0; i < 8; i++)
      printf("%s  0x%x\n", regsb[i], reg_b(i));
  }
  else if(strcmp(arg, "w") == 0) {
    printf("Watchpoint:\n"); 
    print_watchpoint();
  }
  return 0;
}

static int cmd_x(char *args) {
  int num;
  int i;
  vaddr_t addr;
  bool succ = true;
  char *arg = strtok(NULL, " ");
  int ret = atoi(arg);
  if(ret <= 0) {
    printf("invalid input\n");
    return 0;
  }
  num = ret;
  char *exp = strtok(NULL, " ");
  //printf("expr:%s\n", expr);
  addr = expr(exp, &succ);
  if(succ == false)
  {
    printf("x: expression error\n");
    assert(0);
  }
  
  printf("Memory:\n"); 
  for(i = 0; i < num; i++) {
    /*
    printf("0x%x:  0x", addr);
    for(j = 0; j < 4; j++) {
      printf("%02x", vaddr_read(addr, 1));
      addr += 1;
    }
    */
    printf("0x%x:  0x", addr);
    printf("%08x", vaddr_read(addr, 4));
    printf("\n");
    addr += 4;
  }

  printf("0x%x:  0x", addr);
  for(i = 0; i < num%4; i++) {
    printf("%02x", vaddr_read(addr, 1));
    addr += 1;
  }
  printf("\n");
  
  return 0;
}

static int cmd_p(char *args) {
  //printf("p:%s\n", args); 
  bool succ = true;
  int res = expr(args, &succ);
  if(succ == false)
    printf("expression EEROR!\n");
  else
    printf("the value of the expr is %d\n", res);

  return 0;
}

static int cmd_d(char *args) {
  char *arg = strtok(NULL, " ");
  int num = atoi(arg);
  WP *wp;
  wp = (WP*) malloc(sizeof(WP));
  if(num < 0)
  {
    printf("invalid input!\n");
    return 0;
  }
  else
  {
    wp->NO = num;
    wp->next = NULL;
    free_wp(wp);
  }
  return 0;
}

static int cmd_w(char *args) {
  set_watchpoint(args);

  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
