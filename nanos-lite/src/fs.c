#include <fs.h>

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t get_dispinfo();
size_t sb_write(const void *buf, size_t offset, size_t len);
size_t sbctl_write(const void *buf, size_t offset, size_t len);
size_t sbctl_read(void *buf, size_t offset, size_t len);


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

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENT, FD_DISPINFO, FD_SB, FD_SBCTL};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
  [FD_EVENT]  = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISPINFO]= {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
  [FD_SB]     = {"/dev/sb", 0, 0, invalid_read, sb_write},
  [FD_SBCTL]  = {"/dev/sbctl", 0, 0, sbctl_read, sbctl_write},
#include "files.h"
};

static int file_len;

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_len = sizeof(file_table) / sizeof(file_table[0]);
  Log("File table length: %d", file_len);
  file_table[FD_FB].size = get_dispinfo();
  Log("Initializing FB size: %d", file_table[FD_FB].size);
}

int fs_open(const char *path, int flags, int mode) {
  for (int i = 0; i < file_len; i ++) {
    if (strcmp(path, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  Log("Couldn't find file: %s", path);
  return -1;
}


size_t fs_read(int fd, void *buf, size_t len) {
  if (fd < 0 || fd >= file_len) {
    panic("[fs_read] Invalid fd: %d", fd);
    return -1;
  }

  size_t disk_off = file_table[fd].disk_offset, open_off = file_table[fd].open_offset;
  size_t size = file_table[fd].size;

  if (file_table[fd].read) 
    return file_table[fd].read(buf, disk_off + open_off , len);

  size_t read_len = (open_off + len > size) ? (size - open_off) : len;
  size_t ret = ramdisk_read(buf, disk_off + open_off, read_len);
  file_table[fd].open_offset += ret;
  return ret;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  if (fd < 0 || fd >= file_len) {
    panic("[fs_write] Invalid fd: %d", fd);
    return -1;
  }

  size_t disk_off = file_table[fd].disk_offset, open_off = file_table[fd].open_offset;
  size_t size = file_table[fd].size;

  if (file_table[fd].write) 
   return file_table[fd].write(buf, disk_off + open_off, len);

  size_t write_len = (open_off + len > size)? (size - open_off) : len;
  size_t ret = ramdisk_write(buf, disk_off + open_off, write_len);
  file_table[fd].open_offset += ret;
  return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if (fd < 0 || fd >= file_len) {
    panic("[fs_lseek] Invalid fd: %d", fd);
    return -1;
  }

  switch (whence) {
    case SEEK_SET:
      file_table[fd].open_offset = offset;
      break;
    case SEEK_CUR:
      file_table[fd].open_offset += offset;
      break;
    case SEEK_END:
      file_table[fd].open_offset = file_table[fd].size + offset;
      break;
    default:
      panic("[fs_lseek] Invalid whence: %d", whence);
      return -1;
  }

  if (file_table[fd].open_offset > file_table[fd].size) {
    panic("[fs_lseek] seek beyond size");
    return -1;
  }

  return file_table[fd].open_offset;
}

char *get_filename(int fd) {
  if (fd < 0 || fd >= file_len) return NULL;
  else return file_table[fd].name;
}