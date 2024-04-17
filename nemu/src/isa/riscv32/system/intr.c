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


enum {
  mstatus, misa, meedleg, mideleg, mie, mtvec, mcounteren, mstatush, 
  mscratch, mepc, mcause, mtval, mip, mtinst, mtval12
} Machine_Trap;

word_t trap_csr[15] = {};

#define CSRs(csr) trap_csr[csr < 15 ? csr : addr2csr(csr)]

int addr2csr(word_t addr) {
  if (addr >= 0x300 && addr <= 0x306) return addr - 0x300;
  if (addr >= 0x340 && addr <= 0x344) return addr - 0x340 + 8;
  switch (addr) {
    case 0x310: return 7;
    case 0x34a: return 13;
    case 0x34b: return 14;
    default: return -1;
  }
}


word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */

  CSRs(mepc) = epc;
  CSRs(mcause) = NO;
  return CSRs(mtvec);
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}

word_t get_csr(word_t csr) { return CSRs(csr); }
void set_csr(word_t csr, word_t value) { CSRs(csr) = value; printf("%4x: %08x", csr, CSRs(csr)); }
