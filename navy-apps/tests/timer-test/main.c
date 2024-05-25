#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

int main() {
    struct timeval tv;
    int cnt = 15;
    uint32_t time_new = 0, time_old = 0;
    while(cnt) {
        time_new = NDL_GetTicks();
        if (time_new - time_old < 500)
            continue;
        else {
            printf("cnt: %d\n", cnt--);
            time_old = time_new;
        }
    }
    
    printf("PASS!!!\n");
    return 0;
}