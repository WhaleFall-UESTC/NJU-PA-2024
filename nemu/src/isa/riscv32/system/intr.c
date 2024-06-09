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
#include <device/intr.h>

const word_t IRQ_NO[NR_INTR] = {
  [INTR_TIMER] = 0x80000007,
  [INTR_IODEV] = 0x8000000b
};

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
// word_t trap_csr[4] = {};

void set_trap_csr(int i, word_t value) { 
  switch (i) {
    case mepc: cpu.mepc = value; break;
    case mcause: cpu.mcause = value; break;
    case mstatus: cpu.mstatus = value; break;
    case mtvec: cpu.mtvec = value; break;
    case satp: cpu.satp = value; break;
    default: panic("unknown csr");
  }

  // if (i == satp) {
  //   // Log("set satp: %08x", value);
  //   cpu.satp = value;
  //   return;
  // } else if (i == mstatus) {
  //   cpu.mstatus = value;
  //   return;
  // }
  // trap_csr[i] = value; 
}

word_t get_trap_csr(int i) { 
  switch (i) {
    case mepc: return cpu.mepc;
    case mcause: return cpu.mcause;
    case mstatus: return cpu.mstatus;
    case mtvec: return cpu.mtvec;
    case satp: return cpu.satp;
    default: panic("unknown csr");
  }

  // if (i == satp) {
  //   // Log("get satp: %08x", cpu.satp);
  //   return cpu.satp;
  // } else if (i == mstatus) {
  //   // Log("get mstatus: %08x", cpu.mstatus);
  //   return cpu.mstatus;
  // }
  // return trap_csr[i]; 
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
  // etrace_log(NO, epc);
  // trap_csr[mcause] = NO;
  // trap_csr[mepc] = epc;

  cpu.mcause = NO;
  cpu.mepc = epc;
  
  cpu.mpie = cpu.mie;
  cpu.mie = 0;

  // cpu.mpp = cpu.prv;
  // cpu.prv = 3;

  // if (cpu.mtvec & 0x3) {
  //   word_t base = BITS(cpu.mtvec, 31, 2);
  //   word_t interrupt = BITS(NO, 31, 31);
  //   word_t ecode = BITS(NO, 30, 0);
  //   if (interrupt) return base + (ecode << 2);
  //   else return base;
  // }

  return cpu.mtvec;
  // return trap_csr[mtvec];
}



word_t isa_query_intr() {
  // return INTR_EMPTY;
  if (cpu.mie) {
    for (int i = 0; i < NR_INTR; i++) {
      if (INTR[i]) {
        INTR[i] = false;
        return IRQ_NO[i];
      }
    }
  }
  return INTR_EMPTY;
}
