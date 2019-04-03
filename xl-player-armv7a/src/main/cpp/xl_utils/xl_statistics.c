//
// Created by tianchi on 17-6-26.
//

#include "xl_statistics.h"
#include "xl_clock.h"

xl_statistics *xl_statistics_create(JNIEnv *jniEnv) {
    xl_statistics *statistics = malloc(sizeof(xl_statistics));
    statistics->ret_buffer = malloc(12);
    jobject buf = (*jniEnv)->NewDirectByteBuffer(jniEnv, statistics->ret_buffer, 12);
    statistics->ret_buffer_java = (*jniEnv)->NewGlobalRef(jniEnv, buf);
    (*jniEnv)->DeleteLocalRef(jniEnv, buf);
    return statistics;
}

void xl_statistics_reset(xl_statistics *statistics) {
    statistics->last_update_time = 0;
    statistics->last_update_bytes = 0;
    statistics->last_update_frames = 0;
    statistics->bytes = 0;
    statistics->frames = 0;
}

void xl_statistics_release(JNIEnv *jniEnv, xl_statistics *statistics) {
    (*jniEnv)->DeleteGlobalRef(jniEnv, statistics->ret_buffer_java);
    free(statistics->ret_buffer);
    free(statistics);
}

void xl_statistics_update(xl_statistics *statistics) {
    statistics->last_update_time = xl_clock_get_current_time();
    statistics->last_update_bytes = statistics->bytes;
    statistics->last_update_frames = statistics->frames;
}

int xl_statistics_get_fps(xl_statistics *statistics) {
    int64_t frames = statistics->frames - statistics->last_update_frames;
    int64_t time_diff = xl_clock_get_current_time() - statistics->last_update_time;
    return (int) (frames * 1000000 / time_diff);
}

int xl_statistics_get_bps(xl_statistics *statistics) {
    int64_t bytes = statistics->bytes - statistics->last_update_bytes;
    int64_t time_diff = xl_clock_get_current_time() - statistics->last_update_time;
    return (int) (bytes * 8 * 1000000 / time_diff);
}