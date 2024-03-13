#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NEQ,
  TK_AND,
  TK_OR,
  TK_NOT,
  TK_LOGICAND,
  TK_LOGICOR,
  TK_NEG,
  TK_NUM,
  TK_HEX,
  TK_REG,
  TK_DEREF,
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-",'-'},            // minus
  {"\\*",'*'},          // multi
  {"\\/",'/'},          // div
  {"\\(",'('},          // left paren
  {"\\)",')'},          // right paren
  {"==",TK_EQ},          // equa 
  {"!=",TK_NEQ},          // not equal
  {"!",TK_NOT},            // not
  {"&&",TK_LOGICAND},          // logic and
  {"\\|\\|",TK_LOGICOR},      // logic or
  {"&",TK_AND},            // and
  {"\\|",TK_OR},          // or
  {"0[xX][a-fA-F0-9]+",TK_HEX},     //hex number
  {"[0-9]|([1-9][0-9]*)",TK_NUM},   //number
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)",TK_REG},// reg name
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


int get_priorty(Token tok)
{
    switch(tok.type)
    {
        case TK_NEG:
        case TK_DEREF:
        case TK_NOT: return 1;
        case '*':
        case '/': return 2;
        case '+':
        case '-': return 3;
        case TK_EQ:
        case TK_NEQ: return 4;
        case TK_AND:
        case TK_OR: return 5;
        case TK_LOGICAND:
        case TK_LOGICOR: return 6;
        default: return 1000;
    }
}


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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
            case TK_NOTYPE: break;
            case TK_REG:
            {
                strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
		tokens[nr_token].str[substr_len] = '\0';
		tokens[nr_token].type = rules[i].token_type;
		nr_token++;
		break;
            }

            case TK_NUM:
            {
            }
            case TK_HEX:
            {
                for(int i=0;i<substr_len;i++){
                    tokens[nr_token].str[i] = substr_start[i];
                }
                tokens[nr_token].str[substr_len] = '\0';
            }
            default:
            {
                tokens[nr_token].type = rules[i].token_type;
                nr_token++;
            }
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  if(tokens[0].type == '-') tokens[0].type = TK_NEG;
  for(int i=0;i<nr_token;i++)
  {   
      if(tokens[0].type == '-') tokens[0].type = TK_NEG;
      if(tokens[i].type == '-' && tokens[i-1].type != TK_NUM && tokens[i-1].type != ')')
      {
     	  tokens[i].type = TK_NEG;
      }
      else if(tokens[i].type == '*' && (get_priorty(tokens[i-1])<1000 || i==0))
      {
          tokens[i].type = TK_DEREF;
      }
  }
  printf("%d %d %d",tokens[0].type,tokens[1].type,tokens[2].type);
  return true;
}


bool check_parentheses(int p,int q,bool *success)
{
	//printf("get check ()");
    *success = true;
    if(p>q)
    {
        printf("ERROR");
        return false;
    }
    int level = 0;
    for(int i=p;i<=q;i++)
    {
        if(tokens[i].type == '(')
        {
            level++;
        }
        else if(tokens[i].type == ')')
        {
            level--;
            if(level<0)
            {
		*success = false;
                return false;
            }
	}
    }
    if(level != 0)
    {
	*success = false;
	return false;
    }
    if(tokens[p].type == '('&& tokens[q].type == ')' && level == 0)
    {
        return true;
    }
    else
    {
	return false;
    }
}

int find_dominant_operator(int p,int q)
{
//	printf("get find_dominant") ;
    int min_priorty = -1;
    int op_position = -1;

    int parentheses_count = 0;
    
    for(int i=p;i<=q;i++)
    {
        if(tokens[i].type == '(')
        {
            parentheses_count++;
            continue;
        }
        if(tokens[i].type == ')')
        {
            parentheses_count--;
            continue;
        }
	if(parentheses_count != 0) continue;
	if(!(get_priorty(tokens[i])<7)) continue;
        if(parentheses_count == 0)
        {
            int priorty = get_priorty(tokens[i]);
            if(priorty > min_priorty)
            {
                min_priorty = priorty;
                op_position = i;
            }
        }
    }
	printf("\nthe domin_op index is %d\n",op_position);
    return op_position;
}

int eval(int p,int q, bool *success)
{
    if(p>q)
    {
        printf("Bad expression.\n");
        *success = false;
        return 0;
    }
    else if(p == q)
    {
        *success = true;
        //char *end;
        switch(tokens[p].type)
        {
            case TK_NUM:
            {
                int num;
		        sscanf(tokens[p].str,"%d",&num);
		        return num;
            }
            case TK_HEX:
            {
                int num;
		        sscanf(tokens[p].str,"%x",&num);
		        return num;
            }
            case TK_REG:
            {
		printf("reach here\n");
		printf("%s",&tokens[p].str[1]);
		
                for(int i=0;i<8;i++)
                {
                    if(strcmp(&tokens[p].str[1], regsl[i])==0) return reg_l(i);
                    if(strcmp(&tokens[p].str[1], regsw[i])==0) return reg_w(i);
                    if(strcmp(&tokens[p].str[1], regsb[i])==0) return reg_b(i);
                }
                if(strcmp(&tokens[p].str[1], "$eip")==0) return cpu.eip;
            }
            default:
            {
                *success = false;
                return 0;
            }
	}
    }
    else if(check_parentheses(p,q,success) == true)
    {
        return eval(p+1,q-1,success);
    }
    else
    {
        int op = find_dominant_operator(p,q);
        //vaddr_t addr;
        int val2 = eval(op+1,q,success);
        switch(tokens[op].type)
        {
            case TK_NOT:
            {
                if(*success) return !val2;
                else return 0;
            }
            case TK_NEG:
            {
		        printf("get NEG_pd\n");
                return -val2;
            }
	        case TK_DEREF:
	        {
		        return vaddr_read(val2,4);
	        }    
        }
        if(!*success) return 0;
	    int val1 = eval(p,op-1,success);
        switch(tokens[op].type)
        {
            case '+': return val1+val2;
            case '-': return val1-val2;
            case '*': return val1*val2;
            case '/': 
            {
                if(val2 == 0)
                {
                    printf("0 can not be divided");
                    return -1;
                }
                return val1/val2;
            }
            case TK_EQ: return val1==val2;
            case TK_NEQ: return val1!=val2;
            case TK_AND: return val1&val2;
            case TK_OR: return val1|val2;
            case TK_LOGICAND: return val1&&val2;
            case TK_LOGICOR: return val1||val2;
            default:assert(0);
        }
    }
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  *success = true;
  return eval(0,nr_token-1,success);
}
