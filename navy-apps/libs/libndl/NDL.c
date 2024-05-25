#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
// #include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;
static int canvas_x = 0, canvas_y = 0;

uint32_t NDL_GetTicks()
{
  struct timeval *tv;
  gettimeofday(tv, NULL);
  uint32_t t = tv->tv_sec * 1000 + tv->tv_usec / 1000;
  return t;
}

int NDL_PollEvent(char *buf, int len)
{
  // int fd = open("/dev/events", 0, 0);
  // int ret = read(fd, buf, len);
  // assert(close(fd) == 0);
  // return ret == 0 ? 0 : 1;
  return read(evtdev, buf, len);
}

void NDL_OpenCanvas(int *w, int *h)
{
  if (getenv("NWM_APP"))
  {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1)
    {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  }

  int buf_size = 64;
  char *buf = (char *)malloc(buf_size * sizeof(char));
  // int fd = open("/proc/dispinfo", 0, 0);
  int ret = read(-1, buf, buf_size);
  assert(ret < buf_size);
  // assert(close(fd) == 0);

  int i = 0;
  int width = 0, height = 0;
  char *width_str = strstr(buf, "WIDTH");
  char *height_str = strstr(buf, "HEIGHT");

  if (width_str)
  {
    width_str = strchr(width_str, ':') + 1;
    width = atoi(width_str);
    printf("width: %d\n", width);
  }
  if (height_str)
  {
    height_str = strchr(height_str, ':') + 1;
    height = atoi(height_str);
    printf("height: %d\n", height);
  }
  assert(height != 0 && width != 0);

  free(buf);

  *w = width;
  *h = height;
  canvas_w = width;
  canvas_h = height;
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h)
{
  for (int i = 0; i < h && i + y < canvas_h; i++) {
    lseek(fbdev, ((y + canvas_y + i) * screen_w + (x + canvas_x)) * 4, SEEK_SET);
    write(fbdev, pixels + i * w, 4 * (w < canvas_x - x ? w : canvas_w - x));
  }
  assert(close(fbdev) == 0);
}

void NDL_OpenAudio(int freq, int channels, int samples)
{
}

void NDL_CloseAudio()
{
}

int NDL_PlayAudio(void *buf, int len)
{
  return 0;
}

int NDL_QueryAudio()
{
  return 0;
}

int NDL_Init(uint32_t flags)
{
  if (getenv("NWM_APP"))
  {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit()
{
}
