#include <common.h>
#include <device.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

static bool has_uart = 0;
static bool has_key = 0;

AM_INPUT_KEYBRD_T keybrd_queue[256];
int keybrd_head = 0, keybrd_tail = 0;

void update_keybrd() {
  if (has_key) {
    while(1) {
      AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
      printf("keycode: %d\n", ev.keycode);
      if (ev.keycode != AM_KEY_NONE) {
        // if (ev.keydown) {}
        keybrd_queue[keybrd_tail++] = ev;
        keybrd_tail %= 256;
        if (keybrd_tail == keybrd_head)
          keybrd_head = (keybrd_head + 1) % 256;
        break;
      }
    }
  }
}

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++)
    putch(*((char *)(buf + i)));
  return 0;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  // AM_INPUT_KEYBRD_T in = io_read(AM_INPUT_KEYBRD);
  // if (in.keycode == AM_KEY_NONE) {
  //   *(char *)buf = '\0';
  //   return 0;
  // }
  // return snprintf((char *)buf, len, "%s %s\n", in.keydown ? "kd" : "ku", keyname[in.keydown]);
  if (has_uart);
  if (has_key) {
    update_keybrd();
    if (keybrd_head != keybrd_tail) {
      AM_INPUT_KEYBRD_T ev = keybrd_queue[keybrd_head++];
      keybrd_head %= 256;
      return snprintf((char *)buf, len, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
    }
  }

  return 0;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu_info = io_read(AM_GPU_CONFIG);
  return snprintf((char *)buf, len, "WIDTH: %d\nHEIGHT: %d\n", gpu_info.width, gpu_info.height);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T t = io_read(AM_GPU_CONFIG);
  int w = t.width;

  offset /= 4;
  len /= 4;

  int y = offset / w;
  int x = offset - y * w;

  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
  has_key = io_read(AM_INPUT_CONFIG).present;
}