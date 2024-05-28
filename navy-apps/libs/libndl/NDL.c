#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_x = 0, canvas_y = 0;

static uint32_t timer_start = 0;
uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((uint32_t)tv.tv_sec) * 1000 + ((uint32_t)tv.tv_usec) / 1000 - timer_start;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0, 0);
  int ret = read(fd, buf, len);
  assert(close(fd) == 0);
  return ret;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 || *w > screen_w) *w = screen_w;
  if (*h == 0 || *h > screen_h) *h = screen_h;
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  } else {
    canvas_x = (screen_w - *w) / 2;
    canvas_y = (screen_h - *h) / 2;
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", 0, 0);
  for (int i = 0; i < h; i++) {
    lseek(fd, ((canvas_y + y + i) * screen_w + x + canvas_x) * 4, SEEK_SET);
    write(fd, pixels + i * w, w * 4);
  }
  assert(close(fd) == 0);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

void init_ticks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  timer_start = ((uint32_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

void init_display() {
  int fd = open("/proc/dispinfo", 0, 0);
  char buf[64];
  int nread = read(fd, buf, sizeof(buf));
  assert(nread > 0 && nread < sizeof(buf));

  char *w_s = strstr(buf, "WIDTH");
  char *h_s = strstr(buf, "HEIGHT");

  if (w_s) {
    w_s = strchr(w_s, ':') + 1;
    screen_w = atoi(w_s);
  }

  if (h_s) {
    h_s = strchr(h_s, ':') + 1;
    screen_h = atoi(h_s);
  }

  assert(close(fd) == 0);
  printf("Get screen size %dx%d\n", screen_w, screen_h);
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  init_ticks();
  init_display();
  return 0;
}

void NDL_Quit() {
}
