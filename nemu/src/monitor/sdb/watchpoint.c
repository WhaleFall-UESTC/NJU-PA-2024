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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[32];
  word_t value;

} WP;


void init_wp(WP *wp) {
  memset(wp->expr, 0, 31);
  wp->next = NULL;
  wp->value = 0;
}

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
// head: watchpoint bring used.   free: free watchpoint

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


WP* new_up() {
  if (free_ == NULL) return NULL;

  WP *wp = free_;
  free_ = wp->next;
  init_wp(wp);

  return wp;
}


void free_wp(WP *wp) {
  wp->next = free_;
  free_ = wp;
}


int check_wp() {
  WP* tmp_wp = head;
  word_t tmp_new = 0;
  while (tmp_wp != NULL) {
    tmp_new = expr(tmp_wp->expr, NULL);
    if (tmp_new != tmp_wp->value) {
      printf("Watchpoint %d:\n", tmp_wp->NO);
      printf("Old value: %u\tNew value: %u", tmp_wp->value, tmp_new);
      tmp_wp->value = tmp_new;
      nemu_state.state = NEMU_STOP;
      free(tmp_wp);
      return 0;
    }
    tmp_wp = tmp_wp->next;
  }
  free(tmp_wp);
  return 1;
}

void info_link(WP* l) {
  WP* tmp_wp = l;
  printf("Breakpoints:\n");
  while (tmp_wp!= NULL) {
    printf("[%d]\texpr: %s\n", tmp_wp->NO, tmp_wp->expr);
    tmp_wp = tmp_wp->next;
  }
  free(tmp_wp);
}
void info_head() {info_link(head);}
void info_free_() {info_link(free_);}

void append_wp(char *args) {
  WP *wp = new_up();
  strcpy(wp->expr, args);
  wp->next = head;
  head = wp;
  wp->value = expr(args, NULL);
}

void remove_wp(int no) {
  WP* cur = head;
  WP* prev = NULL;

  while(cur != NULL) {
    if (cur->NO == no) {
      if (prev == NULL) {
        head = cur->next;
      } else {
        prev->next = cur->next;
      }
      free_wp(cur);
      return;
    }
    prev = cur;
    cur = cur->next;
  }
}