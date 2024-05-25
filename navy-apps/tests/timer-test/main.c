#include <stdio.h>
#include <sys/time.h>

int main() {
    struct timeval tv;
    int cnt = 15;
    long time_new = 0, time_old = 0;
    while(cnt) {
        gettimeofday(&tv, NULL);
        time_new = tv.tv_sec * 1000000 + tv.tv_usec;
        if (time_new - time_old < 500000)
            continue;
        else {
            printf("cnt: %d\n", cnt--);
            time_old = time_new;
        }
    }
    
    printf("PASS!!!\n");
    return 0;
}