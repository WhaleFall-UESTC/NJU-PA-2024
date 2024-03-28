/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEX, TK_NEQ, TK_AND, TK_OR, 
              TK_LE, TK_L, TK_GE, TK_G, 
              DEREF

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\==", TK_EQ},        // equal

  {"\\-", '-'}, 
  {"\\*", '*'}, 
  {"\\/", '/'}, 
  {"\\(", '('}, 
  {"\\)", ')'}, 
  {"[0-9]+", TK_NUM},
  {"0x[0-9a-z]+", TK_HEX}, 
  {"0x[0-9A-Z]+", TK_HEX},

  {"!=", TK_NEQ}, 
  {"<=", TK_LE}, 
  {"\\<", TK_L},
  {">=", TK_GE},
  {"\\>", TK_G},

  {"&&", TK_AND},
  {"||", TK_OR}
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

word_t eval(int, int);
word_t tmp_eval;

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
        
        tokens[nr_token].type = rules[i].token_type;

        switch (rules[i].token_type) {
          case TK_NUM:
            if (substr_len < 32) {
              memcpy(tokens[nr_token].str, substr_start, substr_len);
              tokens[nr_token].str[substr_len] = '\0';
              nr_token ++;
              break;
            } 
            else {
              printf("Token too long\n");
              assert(0);
            }          
          
          case TK_NOTYPE:
            break;
          case TK_EQ:
            break;

          default: 
            nr_token++;
            break;
        }
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


bool type_is_op(int type) {
  switch (type) {
  //   case '+': case '-': case '*': case '/':
  //   case TK_EQ: case TK_NEQ: case TK_L: case TK_LE:
  //   case TK_AND: case TK_OR:
  //     return true;
  //   default: return false;
    case '(': case ')': return false;
    default: return true;
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == '*' && (i == 0 || type_is_op(tokens[i - 1].type))) {
      tokens[i].type = DEREF;
    }
  }

  return eval(0, nr_token);
}


int test_make_token(char *arg) {
  return make_token(arg);
}



bool check_parentheses(int p, int q) 
{
  if (tokens[p].type != '(' || tokens[q - 1].type != ')') return false;

  short *stack_brackets = ((short*) calloc(nr_token + 1, sizeof(short)));
  stack_brackets[0] = 1;
  short rsp = 0;
  bool leftmost_matched = false;

  for (int i = p + 1; i < q; i++) {
    switch (tokens[i].type) {
      case '(':
        if (leftmost_matched) return false;
        stack_brackets[++rsp] = 1;
        break;

      case ')':
        if (rsp < 0) return false;
        if (stack_brackets[rsp] == 1) {
          if (--rsp == -1) leftmost_matched = true;
          break;
        }else printf("Invalid expr\n"), assert(0);

      default: continue;
    }
  }

  if (rsp == -1) return true;
  else printf("Invalid expr\n"), assert(0);
}


int choose_op(int p, int q) {
  int op = 0, cnt_bracket, cur_type;
  for (int i = p; i < q; i++) {
    cur_type = tokens[i].type;
    if (cur_type < 256) {
      switch (cur_type) {
        case '+':
        case '-':
          op = i; break;
        case '*':
        case '/':
          op = (op == '+' || op == '-') ? op : i; break;
        case '(':
          cnt_bracket = 1;
          while (cnt_bracket) {
            switch (tokens[++i].type) {
              case '(': cnt_bracket++; break;
              case ')': cnt_bracket--; break;
              default: continue;
            }
          }
          if (i > q) printf("Brackets should be matched"), assert(0);
          break;
      }
    } else {
      switch (cur_type) {
        case TK_EQ: case TK_NEQ: case TK_L: case TK_LE:
        case TK_G: case TK_GE:
          op = i;
        case TK_AND: case TK_OR:
          op = (op != TK_AND && op != TK_OR) ? op : i;
      }
    }
  }

  return op;
}


word_t eval(int p, int q) {
  if (p > q - 1) {
    /* Bad experssion */
    printf("p should be less than q\n"), assert(0);
  }
  else if (p == q - 1) {
    switch (tokens[p].type) {
      case TK_NUM: return atoi(tokens[p].str);
      case TK_HEX: 
        sscanf(tokens[p].str, "%x", &tmp_eval);
        return tmp_eval;
      default: 
        printf("Not a number: %d\n", p); assert(0);
    }
  }
  else if (tokens[p].type == DEREF) {
    int addr = 0;
    sscanf(tokens[p + 1].str, "%x", &addr);
    return vaddr_read(addr, 4);
  }
  else if (check_parentheses(p, q)){
    return eval(p + 1, q - 1);
  }
  else {
    int op = choose_op(p, q);
    int val1 = eval(p, op);
    int val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: switch(tokens[op].type) {
        case TK_EQ: return val1 == val2;
        case TK_NEQ: return val1!= val2;
        case TK_L: return val1 < val2;
        case TK_LE: return val1 <= val2;
        case TK_G: return val1 > val2;
        case TK_GE: return val1 >= val2;
        case TK_AND: return val1 && val2;
        case TK_OR: return val1 || val2;
        default: assert(0);
      }
    }
  }
}

int get_nr() {return nr_token;}


void token_s(int p, int q) {
  printf("tokens(%d, %d):\t", p, q);
  for (int i = p; i < q; i++) {
    if (tokens[i].type == TK_NUM) printf("%s", tokens[i].str);
    else printf("%c", tokens[i].type);
  }
  printf("\n");
}