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
#include <memory/paddr.h>
#include <cpu/cpu.h>

typedef union {
  word_t val;
  struct {
    uint32_t v : 1;
    uint32_t r : 1;
    uint32_t w : 1;
    uint32_t x : 1;
    uint32_t u : 1;
    uint32_t g : 1;
    uint32_t a : 1;
    uint32_t d : 1;
    uint32_t rsw : 2;
    uint32_t ppn0 : 10;
    uint32_t ppn1 : 12;
  };
} pte_t;

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
  return cpu.mode ? MMU_TRANSLATE : MMU_DIRECT;
}


#define vpn(vaddr, i) ((vaddr >> (12 + 10 * i)) & 0x3ff)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
// step1:
  word_t a = cpu.ppn << 12;
  int i = 1;
  pte_t pte;
  int valid = 0;

step2:
  pte.val = paddr_read(a + vpn(vaddr, i) * sizeof(pte_t), sizeof(pte_t));
  if (pte.v == 0) NEMUTRAP(cpu.pc, 114514);
// step3:
  if (pte.v == 0 || (pte.r == 0 && pte.w == 1))
    return MEM_RET_FAIL;
// step4:
  // 目前 pte 合法
  if (pte.r == 1 || pte.x == 1)
    goto step6;
// step5:
  // 目前 pte 指向下一级页表
  if (--i < 0)
    return MEM_RET_FAIL;
  a = (((pte.val) >> 10) << 12) & 0xfffff000;
  goto step2;

step6:
  // 目前 pte 是叶表项
  switch(type) {
    case MEM_TYPE_IFETCH: valid = pte.x; break;
    case MEM_TYPE_READ:   valid = pte.r; break;
    case MEM_TYPE_WRITE:  valid = pte.w; break;
  }
  if (valid == 0)
    return MEM_RET_FAIL;
// step7:
  if (i > 0 && pte.ppn0 != 0) // 没对齐
    return MEM_RET_FAIL;
// step8:
  if (pte.a == 0 || (type == MEM_TYPE_WRITE && pte.d == 0))
    return MEM_RET_FAIL;
// step9:
  // 翻译成功
  paddr_t pgaddr = ((pte.val >> 10) << 12) & 0xfffff000;
  if (i > 0) { // 说明这是个一级页表，组成时还要加上 VPN[0]
    pgaddr = (pgaddr & 0xffc00000) | (vpn(vaddr, 0) << 12);
  }
  // paddr_t paddr = pgaddr | (vaddr & 0x00000fff);
  // // assert(paddr == vaddr);
  // Log("vaddr: %08x\ttranslate paddr: %08x", vaddr, paddr);
  return pgaddr | MEM_RET_OK;
}

