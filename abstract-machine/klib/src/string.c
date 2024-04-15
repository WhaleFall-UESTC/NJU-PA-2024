#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int i;
  for(i = 0; s[i] != '\0'; i++);
  return i;
}

char *strcpy(char *dst, const char *src) {
  int i;
  for(i = 0; src[i]!= '\0'; i++) 
    dst[i] = src[i];

  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  int i;
  for(i = 0; src[i]!= '\0' && i < n; i++) 
    dst[i] = src[i];

  do { dst[i++] = '\0'; } while(i < n);
  return dst;
}

char *strcat(char *dst, const char *src) {
  int len = strlen(dst), i;
  for (i = 0; src[i]!= '\0'; i++)
    dst[i + len] = src[i];

  dst[i + len] = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  int i;
  for (i = 0; s1[i] != '\0'; i++)
    if (s1[i] != s2[i])
      return s1[i] - s2[i];
  
  return (s2[i] == '\0') ? 0 : -s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (int i = 0; i < n; i++) 
    if (s1[i] != s2[i])
      return s1[i] - s2[i];
  
  return 0;
}

void *memset(void *s, int c, size_t n) {
  char _c  = (char)  c;
  char *_s = (char *) s;
  for (int i = 0; i < n; i++)
    _s[i] = _c;

  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *_dst = (char *) dst, *_src = (char *) src;
  char *tmp = (char *) malloc(sizeof(char) * n);

  for (int i = 0; i < n; i++) tmp[i] = _src[i];
  for (int i = 0; i < n; i++) _dst[i] = tmp[i];
  // _dst[n] = '\0';

  free(tmp);
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *dst = (char *)out, *src = (char *)in;
  for (int i = 0; i < n; i++) 
    dst[i] = src[i];
  return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  char *_s1 = (char *) s1, *_s2 = (char *) s2;
  for (int i = 0; i < n; i++) 
    if (_s1[i]!= _s2[i])
      return _s1[i] - _s2[i];
  
  return 0;
}

#endif
