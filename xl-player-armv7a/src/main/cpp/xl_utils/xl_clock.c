//
// Created by gutou on 2017/4/20.
//

#include <malloc.h>
#include <sys/time.h>
#include "xl_clock.h"

uint64_t xl_clock_get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}
xl_clock * xl_clock_creat(){
    xl_clock * clock = (xl_clock *)malloc(sizeof(xl_clock));
    return clock;
}
int64_t xl_clock_get(xl_clock * clock){
    if(clock->update_time == 0){
        return INT64_MAX;
    }
    return clock->pts + xl_clock_get_current_time() - clock->update_time;
}
void xl_clock_set(xl_clock * clock, int64_t pts){
    clock->update_time = xl_clock_get_current_time();
    clock->pts = pts;
}
void xl_clock_free(xl_clock * clock){
    free(clock);
}

void xl_clock_reset(xl_clock * clock){
    clock->pts = 0;
    clock->update_time = 0;
}