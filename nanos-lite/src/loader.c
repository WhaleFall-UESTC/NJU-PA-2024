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

static void printEhdr(Elf_Ehdr ehdr);

static uintptr_t loader(PCB *pcb, const char *filename)
{
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;

  #ifdef __LP64__
  printf("Why you 64?\n");
  #endif

  uint16_t Ehdrsz = 0;
  ramdisk_read(&Ehdrsz, 28, 2);
  printf("Ehdr: %d\tSize: %d\n", sizeof(Elf_Ehdr), ehdr);
  ramdisk_read(&ehdr, 0, Ehdrsz);
  assert(*((uint32_t *)(&ehdr.e_ident)) == 0x464c457f);
  printEhdr(ehdr);

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


static void printEhdr(Elf_Ehdr ehdr) {
  printf("magic = %x\n", *((uint32_t *)(&ehdr.e_ident)));
  printf("e_type = %x\n", ehdr.e_type);
  printf("e_machine = %x\n", ehdr.e_machine);
  printf("e_version = %x\n", ehdr.e_version);
  printf("e_entry = %x\n", ehdr.e_entry);
  printf("e_phoff = %x\n", ehdr.e_phoff);
  printf("e_shoff = %x\n", ehdr.e_shoff);
  printf("e_flags = %x\n", ehdr.e_flags);
  printf("e_ehsize = %x\n", ehdr.e_ehsize);
  printf("e_phentsize = %x\n", ehdr.e_phentsize);
  printf("e_phnum = %x\n", ehdr.e_phnum);
  printf("e_shentsize = %x\n", ehdr.e_shentsize);
  printf("e_shnum = %x\n", ehdr.e_shnum);
  printf("e_shstrndx = %x\n", ehdr.e_shstrndx);
}