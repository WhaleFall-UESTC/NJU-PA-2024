#include <common.h>

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

static int sbsize = 0;
static int screen_H = 0, screen_W = 0;
static bool gpu_cfg = 1;

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++) {
    putch(((char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  } else {
    return snprintf(buf, len, "k%c %s\n", ev.keydown ? 'd' : 'u', keyname[ev.keycode]);
  }
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", screen_W, screen_H);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  assert((offset & 3) == 0 && (len & 3) == 0);
  int x = offset / 4 % screen_W;
  int y = offset / 4 / screen_W;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len / 4, 1, 1);
  return len;
}

size_t get_dispinfo() {
  return screen_H * screen_W * 4;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();

  if (gpu_cfg) {
    AM_GPU_CONFIG_T gpu_info = io_read(AM_GPU_CONFIG);
    screen_W = gpu_info.width;
    screen_H = gpu_info.height;
    Log("Initializing screen %dx%d\n", screen_W, screen_H);
  }

  if (io_read(AM_INPUT_CONFIG).present) {
    Log("Initializing input");
  }
  
}

size_t sb_write(const void *buf, size_t offset, size_t len) {
  io_write(AM_AUDIO_PLAY, (Area){.start = (void *)buf, .end = (void *)(buf + len)});
  return len;
}

size_t sbctl_write(const void *buf, size_t offset, size_t len) {
  assert(len == 12);
  io_write(AM_AUDIO_CTRL, .freq = *(int *)buf, .channels = *(int *)(buf + 4), .samples = *(int *)(buf + 8));
  return len;
}

size_t sbctl_read(void *buf, size_t offset, size_t len) {
  return snprintf(buf, len, "%d", sbsize - io_read(AM_AUDIO_STATUS).count);
}
