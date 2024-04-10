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

#include <common.h>
#include <cpu/decode.h>

extern uint64_t g_nr_guest_inst;

#ifndef CONFIG_TARGET_AM
FILE *log_fp = NULL;

void init_log(const char *log_file)
{
  log_fp = stdout;
  if (log_file != NULL)
  {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable()
{
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) && (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
#endif

#ifdef CONFIG_MTRACE
FILE *mtrace = NULL;
void mtrace_init()
{
  mtrace = fopen("/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/mtrace.txt", "w");
  fprintf(mtrace, "start mtrace\n");
}
void mtrace_vread(vaddr_t addr, int len, word_t data) { fprintf(mtrace, "vread addr: %#08x, len = %d, the result is %08x\n", addr, len, data); }
void mtrace_read(vaddr_t addr, int len) { fprintf(mtrace, "read addr: %#08x, len = %d\n", addr, len); }
void mtrace_write(vaddr_t addr, word_t data, int len) { fprintf(mtrace, "write %.*x to addr: %#08x\n", len, data, addr); }
void mtrace_end() { fclose(mtrace); }
#endif

#ifdef CONFIG_FTRACE
static FILE *ftrace_log = NULL;
static FILE *ftrace_symbols = NULL;
static const char symbols_path[] = "/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/symbols";
static const char ftrace_path[] = "/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/ftrace.txt";

typedef struct
{
  vaddr_t addr;
  char name[32];
} symbol_t;
static symbol_t symbols[64];
static int sptr = 0;

#define LOGSYM(s) fprintf(ftrace_log, "addr: %#08x\tname: %s\n", s.addr, s.name)

void ftrace_init(const char *ftrace_elf)
{
  ftrace_log = fopen(ftrace_path, "w");
  fprintf(ftrace_log, "start ftrace at:\n%s\n\n", ftrace_elf);

  char cmd[256];
  sprintf(cmd, "riscv64-linux-gnu-readelf -s %s > %s", ftrace_elf, symbols_path);
  if (-1 == system(cmd))
    fprintf(ftrace_log, "Error run %s\n", cmd);
  ftrace_symbols = fopen(symbols_path, "r");

  char ch;
  char tmp_addr[9] = {}, tmp_type[5] = {};
  while ((ch = fgetc(ftrace_symbols)) != EOF)
  {
    if (ch == ':')
    {
      fseek(ftrace_symbols, 1, SEEK_CUR);
      if (NULL == fgets(tmp_addr, 9, ftrace_symbols))
        continue;
      fseek(ftrace_symbols, 7, SEEK_CUR);

      if (strcmp(fgets(tmp_type, 5, ftrace_symbols), "FUNC") == 0)
      {
        fseek(ftrace_symbols, 24, SEEK_CUR);
        int i = 0;
        while ((ch = fgetc(ftrace_symbols)) != '\n' && i < 64)
          symbols[sptr].name[i++] = ch;
        sscanf(tmp_addr, "%x", &symbols[sptr].addr);

        LOGSYM(symbols[sptr]);
        sptr++;
      }
    }
  }
  fprintf(ftrace_log, "\n");
}

static int layer = 0;
#define P_LAYERS(l) for(int i = 0; i < l; i++){ fprintf(ftrace_log, "\t"); }

void ftrace_call(symbol_t s) {
  P_LAYERS(layer);
  layer++;
  fprintf(ftrace_log, "call [%s @%#08x]\n", s.name, s.addr);
}

void ftrace_ret(char *name) {
  layer--;
  P_LAYERS(layer);
  fprintf(ftrace_log, "ret [%s]\n", name);
}

static symbol_t call_list[64] = {};
static int cptr = 0;

void ftrace(Decode *s, int type) {
  int idx = 0, flag = 1;
  if (type) {
    for (int i = 0; i < cptr; i ++) 
      if (call_list[i].addr == s->pc) {
        flag = 0;
        break;
      }
    if (flag) {
      call_list[cptr].addr = s->pc;
      for (idx = 0; idx < sptr; idx++)
        if (symbols[idx].addr == s->dnpc) {
          strcpy(call_list[cptr++].name, symbols[idx].name);
          break;
        }
      if (idx == sptr) return;
    }
  } 
  else {
    for (idx = 0; idx < cptr; idx++) 
      if (call_list[idx].addr == s->dnpc - 4) 
        break;
    if (idx == cptr) return;
  }

  fprintf(ftrace_log, "%#08x: ", s->pc);
  (type ? ftrace_call(symbols[idx]) : ftrace_ret(call_list[idx].name));
}
#endif