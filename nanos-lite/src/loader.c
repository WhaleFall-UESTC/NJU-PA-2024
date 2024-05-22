#include <proc.h>
#include <elf.h>

#if defined (__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined (__ISA_X86__)
# define EXPECT_TYPE EM_386
#elif defined (__ISA_MIPS32__)
# define EXPECT_TYPE EM_MIPS
#elif defined (__riscv)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_LOONGARCH32R__)
# define EXPECT_TYPE EM_LOONGARCH32R
#else
# error Unsupported ISA
#endif


#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

#define BUF 256

// static void printEhdr(Elf_Ehdr ehdr);
// static void printPhdr(Elf_Phdr phdr);

static uintptr_t loader(PCB *pcb, const char *filename)
{
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;

  #ifdef __LP64__
  printf("Why you 64?\n");
  #endif

  uint16_t Ehdrsz = 0;
  ramdisk_read(&Ehdrsz, 28, 2);
  // printf("Ehdr: %d\tGet Size: %d\n", sizeof(Elf_Ehdr), Ehdrsz);
  ramdisk_read(&ehdr, 0, Ehdrsz);
  assert(*((uint32_t *)(&ehdr.e_ident)) == 0x464c457f);
  // printEhdr(ehdr);
  uintptr_t entrypoint = (uintptr_t) ehdr.e_entry;

  uint32_t e_phoff = ehdr.e_phoff;
  uint16_t e_phentsize = ehdr.e_phentsize;
  uint16_t e_phnum = ehdr.e_phnum;

  uint32_t base = e_phoff + e_phnum * e_phentsize;


  for (int i = 0; i < e_phnum; i++)
  {
    ramdisk_read(&phdr, e_phoff + i * e_phentsize, e_phentsize);
    
    if ((uint16_t)phdr.p_type != PT_LOAD)
      continue;
    else {
      // printf("\nLoad this segment\n");
      // printPhdr(phdr);
    }

    char buf_tmp[BUF];
    uint32_t filesz = phdr.p_filesz, offset = phdr.p_offset;
    uint32_t nread = filesz, read = 0;
    uintptr_t vaddr = phdr.p_vaddr;
    while (nread)
    {
      read = (nread < BUF) ? nread : BUF;
      ramdisk_read(buf_tmp, base + offset, read);
      nread -= read;
      offset += read;
      memcpy((void *)vaddr, buf_tmp, read);
      vaddr += read;
    }

    memset((void *)vaddr, 0, phdr.p_memsz - filesz);
  }

  return (uintptr_t) entrypoint;
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}


// static void printEhdr(Elf_Ehdr ehdr) {
//   printf("magic = %#08x\n", *((uint32_t *)(&ehdr.e_ident)));
//   printf("e_type = %#08x\n", ehdr.e_type);
//   printf("e_machine = %#08x\n", ehdr.e_machine);
//   printf("e_version = %#08x\n", ehdr.e_version);
//   printf("e_entry = %#08x\n", ehdr.e_entry);
//   printf("e_phoff = %#08x\n", ehdr.e_phoff);
//   printf("e_shoff = %#08x\n", ehdr.e_shoff);
//   printf("e_flags = %#08x\n", ehdr.e_flags);
//   printf("e_ehsize = %#08x\n", ehdr.e_ehsize);
//   printf("e_phentsize = %#08x\n", ehdr.e_phentsize);
//   printf("e_phnum = %#08x\n", ehdr.e_phnum);
//   printf("e_shentsize = %#08x\n", ehdr.e_shentsize);
//   printf("e_shnum = %#08x\n", ehdr.e_shnum);
//   printf("e_shstrndx = %#08x\n", ehdr.e_shstrndx);
// }

// static void printPhdr(Elf_Phdr phdr) {
//   printf("p_type = %#08x\n", phdr.p_type);
//   printf("p_offset = %#08x\n", phdr.p_offset);
//   printf("p_vaddr = %#08x\n", phdr.p_vaddr);
//   printf("p_paddr = %#08x\n", phdr.p_paddr);
//   printf("p_filesz = %#08x\n", phdr.p_filesz);
//   printf("p_memsz = %#08x\n", phdr.p_memsz);
//   printf("p_flags = %#08x\n", phdr.p_flags);
//   printf("p_align = %#08x\n", phdr.p_align);
// }