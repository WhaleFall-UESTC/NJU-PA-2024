#ifndef __NDL_H__
#define __NDL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int NDL_Init(uint32_t flags);
void NDL_Quit();
uint32_t NDL_GetTicks();
void NDL_OpenCanvas(int *w, int *h);
int NDL_PollEvent(char *buf, int len);
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h);
void NDL_OpenAudio(int freq, int channels, int samples);
void NDL_CloseAudio();
int NDL_PlayAudio(void *buf, int len);
int NDL_QueryAudio();

#ifdef __cplusplus
}
#endif

#endif


FILE *fopen(const char *path, const char *mode);
int open(const char *path, int flags, ...);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int execve(const char *filename, char *const argv[], char *const envp[]);