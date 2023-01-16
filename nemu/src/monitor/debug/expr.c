#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include "monitor/monitor.h"

enum {
  TK_NOTYPE = 256, TK_EQ = 257, TK_NUM = 258, TK_HEX = 259, TK_REG = 260, TK_NEQ = 261,
  TK_AND = 262, TK_OR = 263, TK_NEGA = 264, TK_DEREF = 265,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    	// spaces
  {"\\+", '+'},         	// plus
  {"-", '-'},         		// sub
  {"\\*", '*'},         	// multiply
  {"/", '/'},        		// divide
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},
  {"\\$(eax|ebx|ecx|edx|esp|ebp|esi|edi|eip|cx|dx|ax|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|bh|dh)", TK_REG},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},	
  {"!", '!'},		
  {"0|[1-9][0-9]*", TK_NUM}, 	// number,
  {"\\(", '('},			// lparen
  {"\\)", ')'},			// rparen
  {"==", TK_EQ}         	// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

 	if(substr_len > 32)
	  assert(0);
	
	switch(rules[i].token_type)
	{
	  case TK_NOTYPE: 
	    nr_token--;
	    break;
	  case '+':
	  case '-':
	  case '*':
	  case '/':
	  case '(':
	  case ')':
	  case '!':
	  case TK_NEQ:
	  case TK_AND:
	  case TK_OR:
	  case TK_DEREF:
	  case TK_NEGA:
	  case TK_EQ:
	  {
	    tokens[nr_token].type = rules[i].token_type;
	    memset(tokens[nr_token].str, '\0', sizeof(tokens[nr_token].str));
	  }
 	  break;
	  case TK_NUM:
	  {
	    memset(tokens[nr_token].str, '\0', sizeof(tokens[nr_token].str));
	    tokens[nr_token].type = rules[i].token_type;
	    strncpy(tokens[nr_token].str, substr_start, substr_len);
	  }
 	  break;
	  case TK_REG:
	  {
	    memset(tokens[nr_token].str, '\0', sizeof(tokens[nr_token].str));
	    tokens[nr_token].type = rules[i].token_type;
	    strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
	  }
 	  break;
	  case TK_HEX:
	  {
	    memset(tokens[nr_token].str, '\0', sizeof(tokens[nr_token].str));
	    tokens[nr_token].type = rules[i].token_type;
	    strncpy(tokens[nr_token].str, substr_start+2, substr_len-2);
	  }
 	  break;
	  default: assert(0);
	}
	//printf("type:%d str:%s\n", tokens[nr_token].type, tokens[nr_token].str);
	nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q) {
  int temp = 0;
  int i = 0;

  if(p > q)
    assert(0);

  //printf("tokens[p].type: %d\n", tokens[p].type);
  //printf("tokens[q].type: %d\n", tokens[q].type);

  if(tokens[p].type != '(' || tokens[q].type != ')')
    return false;
  
  for(i = p+1; i < q; i++) {
    if(tokens[i].type == '(')
      temp++;
    else if(tokens[i].type == ')')
    {
      if(temp != 0)
        temp--;
      else
        return false;
    }
  }
  if(temp == 0)
    return true;
  else
    return false;
}

int findDominantOp(int p, int q) {
  int op[] = {-1, -1, -1, -1, -1};
  int i;
  int level = 0;

  for(i = p; i <= q; i++)
  {
    //printf("tokens[i].type:%d\n", tokens[i].type);
    if(tokens[i].type == '(')
    {
      level++;
    }
    if(tokens[i].type == ')')
    {
      level--;
    }
    
    if(tokens[i].type != TK_NUM)
    {
      //printf("level:%d\n", level);
      if(level == 0 && (tokens[i].type == TK_AND || tokens[i].type == TK_OR))
	op[0] = i;
      else if(level == 0 && (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ))
	op[1] = i;
      else if(level == 0 && (tokens[i].type == '+' || tokens[i].type == '-'))
	op[2] = i;
      else if(level == 0 && (tokens[i].type == '*' || tokens[i].type == '/'))
	op[3] = i;
      else if(level == 0 && (tokens[i].type == '!' || tokens[i].type == TK_NEGA || tokens[i].type == TK_DEREF))
	op[4] = i;
    }
  }
  for(i = 0; i < 5; i++)
  {
    //printf("i:%d  op[i]:%d\n", i, op[i]);
    if(op[i] != -1)
      return op[i];
  }
  printf("dominant operator find error!\n");
  assert(0);
}

uint32_t eval(int p, int q) {
  uint32_t num; 
  int op;
  int i;
  vaddr_t addr;
  int data;
  int res;
  
  //printf("p:%d, q:%d\n", p, q);
  if(p > q)
    assert(0);
  else if(p == q) 
  {
    if(tokens[p].type == TK_NUM)
    {
      sscanf(tokens[p].str, "%d", &num);
      return num;
    }
    else if(tokens[p].type == TK_HEX)
    {
      sscanf(tokens[p].str, "%x", &num);
      return num;
    }
    else if(tokens[p].type == TK_REG)
    {
      for(i = 0; i < 8; i++)
      {
	if(strcmp(tokens[p].str, regsl[i]) == 0)
	  return reg_l(i);
	if(strcmp(tokens[p].str, regsw[i]) == 0)
	  return reg_w(i);
	if(strcmp(tokens[p].str, regsb[i]) == 0)
	  return reg_b(i);
      }
      if(strcmp(tokens[p].str, "eip") == 0)
	return cpu.eip;
      else
        assert(0);
    }
  }
  else if(check_parentheses(p, q) == true)
  {
    return eval(p+1, q-1);
  }
  else
  {
    op = findDominantOp(p, q);
    switch(tokens[op].type)
    {
      case TK_NEGA: return -eval(p+1, q);
      case TK_DEREF: 
        addr = eval(p+1, q);
	data = vaddr_read(addr, 4);
        return data;
      case '!':
	res = eval(p+1, q);
	if(res != 0)
	  return 0;
	else
	  return 1;
    }

    int val1 = eval(p, op-1);
    int val2 = eval(op+1, q);
    switch(tokens[op].type)
    {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      default: assert(0);
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  int i;

  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  /*
  bool suc = check_parentheses(0, nr_token-1);
  if(suc == false)
    printf("check error\n");
  int index = findDominantOp(1, nr_token-2);
  printf("dominant operator: %d\n", index);
  */

  for(i = 0; i < nr_token; i++)
  {
    if(tokens[i].type == '-')
    {
      if((i != 0) && (tokens[i-1].type == TK_NUM || tokens[i-1].type == ')'))
	tokens[i].type = '-';
      else
	tokens[i].type = TK_NEGA;
    }
    if(tokens[i].type == '*')
    {
      if((i != 0) && (tokens[i-1].type == TK_NUM || tokens[i-1].type == ')'))
	tokens[i].type = '*';
      else
	tokens[i].type = TK_DEREF;
    }
  }
  *success = true;
  return eval(0, nr_token-1);

  return 0;
}
