#include <proc.h>
#include <elf.h>
#include <fs.h>
#include <memory.h>
#include <am.h>

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
#define Elf_Word Elf64_Word
#define Elf_Half Elf64_Half
#define Elf_Off Elf64_Off
#define Elf_Addr Elf64_Addr
#define Elf_Memz Elf64_Xword
#define EhdrSize 0x34
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#define Elf_Word Elf32_Word
#define Elf_Half Elf32_Half
#define Elf_Off Elf32_Off
#define Elf_Addr Elf32_Addr
#define Elf_Memz Elf32_Word
#define EhdrSize 0x28
#endif

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#endif

#define BUF 256

// static void printEhdr(Elf_Ehdr ehdr);
// static void printPhdr(Elf_Phdr phdr);

static uintptr_t loader(PCB *pcb, const char *filename)
{
  Elf_Ehdr ehdr;
  Elf_Phdr phdr;

  #ifdef __LP64__
  Log("Loading Elf64");
  #endif

  int fd = fs_open(filename, 0, 0);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  // printEhdr(ehdr);
  // ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert(*((uint32_t *)(&ehdr.e_ident)) == 0x464c457f);
  Elf_Addr entrypoint = ehdr.e_entry;

  Elf_Off e_phoff = ehdr.e_phoff;
  Elf_Half e_phentsize = ehdr.e_phentsize;
  Elf_Half e_phnum = ehdr.e_phnum;

  for (int i = 0; i < e_phnum; i++)
  {
    // ramdisk_read(&phdr, e_phoff + i * e_phentsize, e_phentsize);
    fs_lseek(fd, e_phoff + i * e_phentsize, SEEK_SET);
    fs_read(fd, &phdr, e_phentsize);

    if ((Elf_Word)phdr.p_type != PT_LOAD)
      continue;
    

    Elf_Memz memsz = phdr.p_memsz;
    Elf_Memz nread = PGSIZE, pt;
    Elf_Off phoff = phdr.p_offset;
    void *pa = NULL;
    void *va = (void *)phdr.p_vaddr;

    int port = (phdr.p_flags & PF_R ? MMAP_READ : 0)  |
               (phdr.p_flags & PF_W ? MMAP_WRITE : 0) |  
               (phdr.p_flags & PF_X ? MMAP_EXEC : 0);

    for (pt = 0; pt <= memsz; pt += PGSIZE) {
      if (memsz - pt < PGSIZE)
        nread = memsz - pt;

      pa = new_page(1);
      map(&pcb->as, va, pa, port);
      Log("New page at pa %p, mapped to va %p", pa, va);
      fs_lseek(fd, phoff + pt, SEEK_SET);
      fs_read(fd, pa, nread);
      va += PGSIZE;
    }

    memset(((void *)(pa + nread)), 0, PGSIZE - nread);
    // fs_read(fd, (void *)phdr.p_vaddr, phdr.p_memsz);
    // memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
  }

  Log("Loaded. Get entry: %08x", entrypoint);
  return (uintptr_t) entrypoint;
}

void naive_uload(PCB *pcb, const char *filename)
{
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb->stack, pcb + 1}, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  Log("argv: %p, envp: %p", argv, envp);
  protect(&pcb->as);

  void *page = new_page(NR_USTACKPG);
  void *sp = page + NR_USTACKPG * PGSIZE;

  int argc, envc;
  for (argc = 0; argv[argc] != NULL ; argc++);
  for (envc = 0; envp[envc] != NULL ; envc++);
  Log("Get argc: %d, envc: %d", argc, envc);
  const int const_argc = argc, const_envc = envc;
  char* argv_pt[const_argc];
  char* envp_pt[const_envc];
  for (int i = 0; i < argc; i++) {
    Log("argv[%d]: %s", i, argv[i]);
    sp -= strlen(argv[i]) + 1;
    strcpy(sp, argv[i]);
    argv_pt[i] = sp;
  }

  for (int i = 0; i < envc; i++) {
    Log("envp[%d]: %s", i, envp[i]);
    sp -= strlen(envp[i]) + 1;
    strcpy(sp, envp[i]);
    envp_pt[i] = sp;
  }

  sp -= sizeof(void *);
  *((uintptr_t *)sp) = 0UL;

  for (int i = envc - 1; i >=0; i--) {
    sp -= sizeof(char *);
    *((uintptr_t *)sp) = (uintptr_t) envp_pt[i];
  }

  sp -= sizeof(void *);
  *((uintptr_t *)sp) = 0UL;

  for (int i = argc - 1; i >= 0; i--) {
    sp -= sizeof(char *);
    *((uintptr_t *)sp) = (uintptr_t) argv_pt[i];
  }

  sp -= sizeof(void *);
  *((uintptr_t *)sp) = argc;

  free(argv_pt);
  free(envp_pt);

  uintptr_t entry = loader(pcb, filename);
  if (!entry) {
    pcb->cp = NULL;
    return;
  }

  void *vpage = pcb->as.area.end - NR_USTACKPG * PGSIZE;
  for (int i = 0; i < NR_USTACKPG; i++) {
    map(&pcb->as, vpage + PGSIZE * i, page + PGSIZE * i, MMAP_READ | MMAP_WRITE);
  }

  pcb->cp = ucontext(&pcb->as, (Area) { pcb, pcb + 1 }, (void *)entry);
  pcb->cp->GPRx = (uintptr_t) sp + vpage - page;
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
//   printf("e_ehsize = %d\n", ehdr.e_ehsize);
//   printf("e_phentsize = %d\n", ehdr.e_phentsize);
//   printf("e_phnum = %d\n", ehdr.e_phnum);
//   printf("e_shentsize = %d\n", ehdr.e_shentsize);
//   printf("e_shnum = %d\n", ehdr.e_shnum);
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