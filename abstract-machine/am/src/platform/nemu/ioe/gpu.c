#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define SIZE_MASK 0x0000ffff

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
  uint32_t width = (wh >> 16) & SIZE_MASK;
  uint32_t height = wh & SIZE_MASK;
  uint32_t size = width * height;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = size
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pixels = (uint32_t *)ctl->pixels;
  uint32_t width = (inl(VGACTL_ADDR) >> 16) & SIZE_MASK;

  int base_fb = x * width + y;
  int base_pixels = 0;
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      fb[base_fb + j] = pixels[base_pixels + j];
    }
    base_fb += width;
    base_pixels += w;
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
