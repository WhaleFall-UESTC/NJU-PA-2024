#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <NDL.h>
#include <BMP.h>

int main() {
  // printf("Start, enter main()\n");
  NDL_Init(0);
  // printf("Init\n");
  int w, h;
  void *bmp = BMP_Load("/share/pictures/projectn.bmp", &w, &h);
  assert(bmp);
  // printf("Load\n");
  NDL_OpenCanvas(&w, &h);
  // printf("Open\n");
  NDL_DrawRect(bmp, 0, 0, w, h);
  // printf("Draw\n");
  printf("Test ends! Spinning...\n");
  free(bmp);
  printf("free\n");
  NDL_Quit();
  // while (1);
  return 0;
}
