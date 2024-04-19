#include <proc.h>
#include <elf.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

#define BUF 256

static uintptr_t loader(PCB *pcb, const char *filename)
{
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;

  ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert(*((uint32_t *)(&ehdr.e_ident)) == 0x464c457f);

  uint16_t e_phoff = ehdr.e_phoff;
  uint16_t e_phentsize = ehdr.e_phentsize;
  uint16_t e_phnum = ehdr.e_phnum;

  for (int i = 0; i < e_phnum; i++)
  {
    ramdisk_read(&phdr, e_phentsize, e_phoff + i * e_phentsize);
    if (phdr.p_type != PT_LOAD)
      continue;

    char buf_tmp[BUF];
    uint32_t filesz = phdr.p_filesz, offset = phdr.p_offset;
    uint32_t nread = filesz, read = 0;
    uint32_t vaddr = phdr.p_vaddr;
    while (nread)
    {
      read = (nread < BUF) ? nread : BUF;
      ramdisk_read(buf_tmp, read, offset);
      nread -= read;
      offset += read;
      memcpy((void *)vaddr, buf_tmp, read);
      vaddr += read;
    }

    memset((void *)vaddr, 0, phdr.p_memsz - filesz);
  }

  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
