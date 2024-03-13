#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args)
{
  return -1;
}

static int cmd_help(char *args);

/*PA 1.1.2 implement single-step execution,print register,scan the memory */
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct
{
  char *name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    //single step debug
    {"si", "Execute step by step", cmd_si},
    {"info", "Print program information", cmd_info},
    {"x", "Scan memory", cmd_x},
    {"p", "Compute the expression", cmd_p},
    {"w", "apply new watchpointer", cmd_w},
    {"d", "delete No.n watchpointer", cmd_d},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

//single-step execution
static int cmd_si(char *args)
{
  char *arg = strtok(NULL, " ");
  int step = 1; //default step = 1
  if (arg != NULL)
  {
    sscanf(arg, "%d", &step);
  }
  //if step is not a postive number , print warning
  if (step <= 0)
  {
    printf("Invalid step count. Please enter a postive integer.(step > 0)\n");
  }
  //execute step
  for (int i = 0; i < step; i++)
  {
    cpu_exec(1);
  }
  return 0;
}

//print information
static int cmd_info(char *args)
{
  char *arg = strtok(args, " ");
  // if subcmd is r, print reg informations
  if (strcmp(arg, "r") == 0)
  {
    printf("\n32-bit registers:\n");
    for (int i = 0; i < 8; i++)
    {
      printf("%s = 0x%08x\n", regsl[i], reg_l(i));
    }
    printf("eip = 0x%08x\n", cpu.eip);

    printf("\n16-bit registers:\n");
    for (int i = 0; i < 8; i++)
    {
      printf("%s = 0x%04x\n", regsw[i], reg_w(i));
    }

    printf("\n8-bit registers:\n");
    for (int i = 0; i < 8; i++)
    {
      if (i < 4)
      {
        //low bit
        printf("%s = 0x%02x\n", regsb[i], reg_b(i * 2));
      }
      else
      {
        //high bit
        printf("%s = 0x%02x\n", regsb[i], reg_b((i - 4) * 2 + 1));
      }
    }
  }
  else if (strcmp(arg, "w") == 0)
  {
    print_wp();
  }
  else
    printf("unknown cmd");
  return 0;
}

//Scan memory formation : x N expr
static int cmd_x(char *args)
{
  char *N = strtok(NULL, " ");
  char *exprr = strtok(NULL, " ");
  //check whether the args are existed
  if (N == NULL && exprr == NULL)
  {
    printf("Usage: x [N] [expr]\n");
    return 0;
  }

  int length;
  vaddr_t addr;
  sscanf(N, "%d", &length);
  bool success;
  addr = expr(exprr, &success);
  if (!success)
    printf("ERROR:expression is not right\n");
  for (int i = 0; i < length; i++)
  {
    uint32_t data;
    data = vaddr_read(addr + 4 * i, 4);
    if (i % 4 == 0)
    {
      if (i > 0)
        printf("\n");
      printf("0x%08x: ", addr + 4 * i);
    }
    printf("%08x ", data);
  }
  printf("\n");
  return 0;
}

//Compute the expression
static int cmd_p(char *args)
{
  bool success;
  int result = expr(args, &success);
  if (success == 0)
    printf("EEROR\n");
  else
  {
    printf("The expression value is %d\n", result);
  }
  return 0;
}

static int cmd_w(char *args)
{
  if (args == NULL)
  {
    printf("ERROR : args can not be NULL\n");
    return 0;
  }
  char *expression = args;
  char *buffer = (char *)calloc(strlen(args) + 1, sizeof(char));
  if (!buffer)
  {
    printf("malloc error\n");
    return 0;
  }

  int buffer_length = 0;
  char *token = strtok(expression, " ");
  while (token)
  {
    strcpy(buffer + buffer_length, token);
    buffer_length += strlen(token);
    token = strtok(NULL, " ");
  }

  bool success;
  WP *wp = new_wp();
  if (!wp)
  {
    free(buffer);
    printf("ERROR: can not create new watchpointer\n");
    return 0;
  }

  wp->expr = buffer;
  wp->value = expr(buffer, &success);
  if (!success)
  {
    free_wp(wp->NO);
    printf("ERROR: evaluate expression failed\n");
    return 0;
  }
  printf("already set new watchpoint %d,expression %s,value %d \n", wp->NO, wp->expr, wp->value);
  return 0;
}

static int cmd_d(char *args)
{
  if (args == NULL && strtok(NULL, " ") != NULL)
  {
    printf("ERROR : args error\n");
    return 0;
  }
  char *endptr;
  long n = strtol(args, &endptr, 10);
  if (*endptr != '\0' || endptr == args)
  {
    printf("Number is useless\n");
    return 0;
  }
  if (n >= 32)
  {
    printf("ERROR: the no is out of range\n");
    return 0;
  }

  bool is_freed = free_wp((int)n);
  if (is_freed)
  {
    printf("delete already\n");
  }
  else
  {
    printf("delete failed\n");
  }
  return 0;
}

void ui_mainloop(int is_batch_mode)
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  while (1)
  {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}
