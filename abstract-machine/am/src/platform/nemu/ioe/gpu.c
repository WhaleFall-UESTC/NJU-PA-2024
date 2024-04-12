#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define SIZE_MASK 0x0000ffff

void __am_gpu_init() {
  uint32_t s = inl(VGACTL_ADDR);
  int w = (s >> 16) & SIZE_MASK;
  int h = s & SIZE_MASK;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int size = w * h;
  for (int i = 0; i < size; i++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t s = inl(VGACTL_ADDR);
  uint32_t w = (s >> 16) & SIZE_MASK, h = s & SIZE_MASK;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = w * h
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
