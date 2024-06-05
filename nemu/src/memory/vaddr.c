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
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <cpu/cpu.h>

extern int isa_mmu_check(vaddr_t vaddr, int len, int type);
extern paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type);

word_t vaddr_read_r(vaddr_t addr, int len, int type) {
  if (isa_mmu_check(addr, len, type) == MMU_TRANSLATE) {
    paddr_t res = isa_mmu_translate(addr, len, type);
    if ((res & PAGE_MASK) == MEM_RET_OK) {
      paddr_t pa = (res & ~PAGE_MASK) | (addr & PAGE_MASK);
      return paddr_read(pa, len);
    }
    Log("vaddr_read_r: error vaddr = %08x, len=%d, type=%d, mmu ret: %08x", addr, len, type, res);
    set_nemu_state(NEMU_ABORT, cpu.pc, ABORT_MEMIO);
    return paddr_read(addr, len);
  }
  return paddr_read(addr, len);
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return vaddr_read_r(addr, len, MEM_TYPE_IFETCH);
}

void mtrace_vread(vaddr_t, int, word_t);

word_t vaddr_read(vaddr_t addr, int len) {
  word_t res = vaddr_read_r(addr, len, MEM_TYPE_READ);
  IFDEF(CONFIG_MTRACE, mtrace_vread(addr, len, res));
  return res;
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  if (isa_mmu_check(addr, len, MEM_TYPE_WRITE) == MMU_TRANSLATE) {
    paddr_t res = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
    if ((res & PAGE_MASK) == MEM_RET_OK) {
      paddr_t pa = (res & ~PAGE_MASK) | (addr & PAGE_MASK);
      return paddr_write(pa, len, data);
    }
    Log("vaddr_write: error vaddr = %08x, len=%d, type=%d, mmu ret: %08x", addr, len, MEM_TYPE_WRITE, res);
    set_nemu_state(NEMU_ABORT, cpu.pc, ABORT_MEMIO);
    return paddr_write(addr, len, data);
  }
  return paddr_write(addr, len, data);
}