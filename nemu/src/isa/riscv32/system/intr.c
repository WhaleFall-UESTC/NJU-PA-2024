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
#include <utils.h>
#include <cpu/cpu.h>

#define IRQ_TIMER 0x80000007


// enum {
//   mstatus, misa, meedleg, mideleg, mie, mtvec, mcounteren, mstatush, 
//   mscratch, mepc, mcause, mtval, mip, mtinst, mtval12
// } Machine_Trap;

// word_t trap_csr[15] = {};

// #define CSRs(csr) trap_csr[csr < 15 ? csr : addr2csr(csr)]

// int addr2csr(word_t addr) {
//   if (addr >= 0x300 && addr <= 0x306) return addr - 0x300;
//   if (addr >= 0x340 && addr <= 0x344) return addr - 0x340 + 8;
//   switch (addr) {
//     case 0x310: return 7;
//     case 0x34a: return 13;
//     case 0x34b: return 14;
//     default: return -1;
//   }
// }

enum { mepc, mcause, mstatus, mtvec, satp };
word_t trap_csr[4] = {};

void set_trap_csr(int i, word_t value) { 
  if (i == satp) {
    // Log("set satp: %08x", value);
    cpu.satp = value;
    return;
  } else if (i == mstatus) {
    cpu.mstatus = value;
    return;
  }
  trap_csr[i] = value; 
}
word_t get_trap_csr(int i) { 
  if (i == satp) {
    // Log("get satp: %08x", cpu.satp);
    return cpu.satp;
  } else if (i == mstatus) {
    // Log("get mstatus: %08x", cpu.mstatus);
    return cpu.mstatus;
  }
  return trap_csr[i]; 
}


int csr_register(word_t imm) {
  if (imm == 0x180) {
    // Log("Detect satp");
    return satp;
  }
  switch (imm) {
    case 0x341: return mepc; //&(cpu.csr.mepc);
    case 0x342: return mcause; //&(cpu.csr.mcause);
    case 0x300: return mstatus; //&(cpu.csr.mstatus);
    case 0x305: return mtvec; //&(cpu.csr.mtvec);
    default: panic("unknown csr");
  }
}

word_t ecall(word_t sys_call, vaddr_t epc) {
  switch(sys_call) {
    case -1: return isa_raise_intr(1, epc); // EVENT_YIELD
    default: return isa_raise_intr(sys_call, epc);
  }
}

void etrace_log(int, word_t);

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  etrace_log(NO, epc);
  trap_csr[mcause] = NO;
  trap_csr[mepc] = epc;
  
  cpu.mpie = cpu.mie;
  cpu.mie = 0;

  return trap_csr[mtvec];
}



word_t isa_query_intr() {
  // return INTR_EMPTY;
  if (cpu.mie) {
    cpu.INTR = 0;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}
