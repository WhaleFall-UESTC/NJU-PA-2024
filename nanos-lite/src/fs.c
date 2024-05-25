#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

// size_t 

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0/*, invalid_read, invalid_write*/},
  [FD_STDOUT] = {"stdout", 0, 0/*, invalid_read, invalid_write*/},
  [FD_STDERR] = {"stderr", 0, 0/*, invalid_read, invalid_write*/},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}


int fs_open(const char *filename) {
  int i;
  for (i = 0; i < sizeof(file_table) / sizeof(file_table[0]); i ++) {
    if (strcmp(filename, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  if (fd <= 2){
    Log("[fs_read] fd should be greater than 2");
    return 0;
  } 

  size_t file_size = file_table[fd].size;
  size_t file_offset = file_table[fd].disk_offset;
  size_t open_offset = file_table[fd].open_offset;
  size_t read_len = len;

  if (open_offset > file_size) {
    return 0;
  } else if (open_offset + len > file_size) {
    read_len = file_size - open_offset;
  }

  ramdisk_read(buf, file_offset + open_offset, read_len);
  file_table[fd].open_offset += read_len;
  return read_len;
}

size_t fs_write(int fd, void *buf, size_t count) {
  if (fd == 0) {
    Log("[fs_write] fd = 0, ret 0");
    return 0;
  }
  else if (fd == 1 || fd == 2) {
    for (size_t i = 0; i < count; i++)
      putch(*((char *)(buf + i)));
    return 0;
  }

  size_t file_size = file_table[fd].size;
  size_t file_offset = file_table[fd].disk_offset;
  size_t open_offset = file_table[fd].open_offset;
  size_t write_len = count;

  if (open_offset > file_size) {
    return 0;
  } else if (open_offset + write_len > file_size) {
    write_len = file_size - open_offset;
  }

  ramdisk_write(buf, file_offset + open_offset, write_len);
  file_table[fd].open_offset += write_len;
  return write_len;
}

// 这个文件系统没有所谓打开状态
int fs_close() {
  return 0;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if (fd <= 2) {
    Log("[fs_lseek] fd should be greater than 2");
    return 0;
  }

  size_t file_size = file_table[fd].size;
  size_t open_offset = file_table[fd].open_offset;

  switch(whence) {
    case SEEK_SET:
      if (offset > file_size) {
        return 0;
      }
      file_table[fd].open_offset = offset;
      break;
    case SEEK_CUR:
      if (open_offset + offset > file_size) {
        return 0;
      }
      file_table[fd].open_offset += offset;
      break;
    case SEEK_END:
      if (offset > file_size) {
        return 0;
      }
      file_table[fd].open_offset = file_size + offset;
      break;
    default:
      Log("[fs_lseek] invalid whence");
      return -1;
  }

  return file_table[fd].open_offset;
}