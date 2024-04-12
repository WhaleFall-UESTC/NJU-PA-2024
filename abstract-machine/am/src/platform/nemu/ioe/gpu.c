#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define SIZE_MASK 0x0000ffff


void __am_gpu_init() {
  uint32_t wh = inl(VGACTL_ADDR);
  int w = (wh >> 16) & SIZE_MASK;
  int h = wh & SIZE_MASK;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int s = w * h;
  for (int i = 0; i < s; i++) fb[i] = 0x00ffffff;
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
  // if (ctl->sync) {
  //   int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  //   if (w == 0 || h == 0) return;
  //   uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  //   uint32_t *pixels = (uint32_t *) ctl->pixels;
  //   uint32_t width = inl(VGACTL_ADDR) >> 
    
  //   fb += (x * width + y);
  //   for (int i = 0; i < h; i++) {
  //     for (int j = 0; j < w; j++) {
  //       fb[j] = *pixels++;
  //     }
  //     fb += width;
  //   }
  //   outl(SYNC_ADDR, 1);
  // }

  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if (!ctl->sync && (w == 0 || h == 0))
    return;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  for (int i = y; i < y+h; i++) {
    for (int j = x; j < x+w; j++) {
      fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)]; //缓冲区是一个像素块
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);    //将sync置1，nemu会进行屏幕更新
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
