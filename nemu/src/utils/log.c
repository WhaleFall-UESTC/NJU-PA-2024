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
#include <string.h>

extern uint64_t g_nr_guest_inst;

#ifndef CONFIG_TARGET_AM
FILE *log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  Log("Log is written to %s", log_file ? log_file : "stdout");


}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
#endif

#ifdef CONFIG_MTRACE
FILE *mtrace = NULL;
void mtrace_init() {
  mtrace = fopen("/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/mtrace.txt", "w"); 
  fprintf(mtrace, "start mtrace\n");
}
void mtrace_vread(vaddr_t addr, int len, word_t data) {fprintf(mtrace, "vread addr: %#08x, len = %d, the result is %08x\n", addr, len, data);}
void mtrace_read(vaddr_t addr, int len) {fprintf(mtrace, "read addr: %#08x, len = %d\n", addr, len);}
void mtrace_write(vaddr_t addr, word_t data, int len) {fprintf(mtrace, "write %.*x to addr: %#08x\n", len, data, addr);}
void mtrace_end() {fclose(mtrace);} 
#endif


#ifdef CONFIG_FTRACE
FILE *ftrace_log = NULL;
FILE *ftrace_symbols = NULL;
const char symbols_path[] = "/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/symbols";
const char ftrace_path[] = "/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/ftrace.txt";

typedef struct{ vaddr_t addr; char name[32]; } symbol_t;
symbol_t symbols[64];
int sptr = 0;

void ftrace_init(const char *ftrace_elf) {
  ftrace_log = fopen(ftrace_path, "w");
  fprintf(ftrace_log, "start ftrace at:\n%s\n\n", ftrace_elf);

  char cmd[256];
  sprintf(cmd, "riscv64-linux-gnu-readelf -s %s > %s", ftrace_elf, symbols_path);
  if (-1 == system(cmd)) fprintf(ftrace_log, "Error run %s\n", cmd);
  ftrace_symbols = fopen(symbols_path, "r");
  
  char ch;
  char tmp[9] = {};
  while((ch = fgetc(ftrace_symbols)) != EOF) {
    if (ch == ':') {
      fseek(ftrace_symbols, 1, SEEK_CUR);
      if (NULL == fgets(tmp, 9, ftrace_symbols)) continue;
      fseek(ftrace_symbols, 7, SEEK_CUR);
      
      if (NULL != fgets(tmp, 5, ftrace_symbols))
        printf("%s\n", tmp);
        

        // fseek(ftrace_symbols, 23, SEEK_CUR);
        // printf("%s\n", fgets(tmp ,4, ftrace_symbols));
        // int i = 0;
        // while((ch = fgetc(ftrace_symbols)) != '\n') {
        //   symbols[sptr].name[i++] = ch;
        // }
        // printf("%s: %#08x\n\n", symbols[sptr].name, symbols[sptr].addr);
        // sptr++;
      // }
      
      // for (int i = 0; i < 4; i++) 
      //   tmp[i] = fgetc(ftrace_symbols);
      // if (strncmp(tmp, "FUNC", 4) != 0)
      //   continue;

      // sscanf(tmp, "%x", &symbols[sptr].addr);
      // printf("%s %#08x\n", tmp, symbols[sptr].addr);
      // for(int i = 0; i < 8; i++) {
      //   addr_tmp[i] = fgetc(ftrace_symbols);
      // }
      
      // fseek(ftrace_symbols, 7, SEEK_CUR);

    }
  }
  printf("end\n");
}
#endif