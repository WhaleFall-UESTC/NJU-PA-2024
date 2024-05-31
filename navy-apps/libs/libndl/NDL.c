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
static int sbdev = -1, sbctldev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_x = 0, canvas_y = 0;

static uint32_t timer_start = 0;
uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return ((uint32_t)tv.tv_sec) * 1000 + ((uint32_t)tv.tv_usec) / 1000 - timer_start;
}

int NDL_PollEvent(char *buf, int len) {
  return read(evtdev, buf, len);
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
  for (int i = 0; i < h; i++) {
    lseek(fbdev, ((canvas_y + y + i) * screen_w + x + canvas_x) * 4, SEEK_SET);
    write(fbdev, pixels + i * w, w * 4);
  }
}


void NDL_OpenAudio(int freq, int channels, int samples) {
  int spec[3] = {freq, channels, samples};
  write(sbctldev, spec, sizeof(spec));
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return write(sbdev, buf, len);
}

int NDL_QueryAudio() {
  char buf[16];
  read(sbctldev, buf, sizeof(buf));
  return atoi(buf);
}

void init_ticks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  timer_start = ((uint32_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

void init_display() {
  char buf[64];
  int fd = open("/proc/dispinfo", 0, 0);
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

  printf("Get screen size %dx%d\n", screen_w, screen_h);
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  fbdev = open("/dev/fb", 0, 0);
  evtdev = open("/dev/events", 0, 0);
  sbdev = open("/dev/sb", 0, 0);
  sbctldev = open("/dev/sbctl", 0, 0);

  init_ticks();
  init_display();
  return 0;
}

void NDL_Quit() {
}
