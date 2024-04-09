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
void mtrace_init() {mtrace = fopen("/home/whalefall/Courses/NJU-PA/ics2023/nemu/mylog/mtrace.txt", "w"); fprintf(mtrace, "start mtrace\n");}
void mtrace_vread(vaddr_t addr, int len, word_t data) {fprintf(mtrace, "vread addr: %#08x, len = %d, the result is %08x\n", addr, len, data);}
void mtrace_read(vaddr_t addr, int len) {fprintf(mtrace, "read addr: %#08x, len = %d\n", addr, len);}
void mtrace_write(vaddr_t addr, word_t data, int len) {fprintf(mtrace, "write %.*x to addr: %#08x\n", len, data, addr);}
void mtrace_end() {fclose(mtrace);} 
#endif