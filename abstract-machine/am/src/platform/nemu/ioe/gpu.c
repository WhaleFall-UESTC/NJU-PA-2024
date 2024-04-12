#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define SIZE_MASK 0x0000ffff

static uint32_t size = 0, width = 0, height = 0;

void __am_gpu_init() {
  uint32_t wh = inl(VGACTL_ADDR);
  int w = (wh >> 16) & SIZE_MASK;
  int h = wh & SIZE_MASK;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int s = w * h;
  for (int i = 0; i < s; i++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t wh = inl(VGACTL_ADDR);
  width = (wh >> 16) & SIZE_MASK;
  height = wh & SIZE_MASK;
  size = width * height;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = size
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);

    uint32_t *start = (uint32_t *)(uintptr_t)FB_ADDR;
    start += (ctl->x * width + ctl->y);
    uint32_t *store = (uint32_t *)(ctl->pixels);
    for (int i = 0; i < ctl->h; i++) {
      for (int j = 0; j < ctl->w; j++) {
        start[j] = *store;
      }
      start += width;
    }
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
