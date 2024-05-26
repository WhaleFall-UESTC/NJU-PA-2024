#include <fs.h>
#include <common.h>
#include <device.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_FB, FD_DINFO};

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
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENT]  = {"/dev/event", 0, 0, events_read, invalid_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_DINFO]  = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T t = io_read(AM_GPU_CONFIG);
  int width = t.width;
  int height = t.height;
  file_table[FD_FB].size = width * height * 4;
}

int fs_open(const char *filename) {
  // if (strcmp(filename, "/proc/dispinfo") == 0) {
  //   return FD_DISPINFO;
  // } else if (strcmp(filename, "/dev/fb") == 0) {
  //   return FD_FB;
  // }

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
  // if (fd == FD_DISPINFO)
  //   return dispinfo_read(buf, 0, len);

  ReadFn read_fn = file_table[fd].read;
  if (read_fn != NULL) {
    return read_fn(buf, 0, len);
  }

  // if (fd <= 2){
  //   Log("[fs_read] fd should be greater than 2");
  //   return 0;
  // } 

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
  // if (fd == 0) {
  //   Log("[fs_write] fd = 0, ret 0");
  //   return 0;
  // } else if (fd == FD_FB) {
  //   return fb_write(buf, 0, count);
  // } 

  WriteFn write_fn = file_table[fd].write;
  if (write_fn != NULL) {
    return write_fn(buf, 0, count);
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
    // Log("[fs_lseek] fd should be greater than 2");
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

char *get_filename(int fd) {
  switch (fd) {
    case FD_STDIN:
      return "/dev/stdin";
    case FD_STDOUT:
      return "/dev/stdout";
    case FD_STDERR:
      return "/dev/stderr";
    default:
      return file_table[fd].name;
  }
}