#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  char *buf = (char *) malloc(4 * sizeof(char));
  write(1, "Hello World!\n", 13);
  printf("Hello, %s\n", buf);
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }
  return 0;
}
