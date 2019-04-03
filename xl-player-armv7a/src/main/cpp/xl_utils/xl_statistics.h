//
// Created by tianchi on 17-6-26.
//

#ifndef XL_XL_STATISTICS_H
#define XL_XL_STATISTICS_H
#include "../xl_types/xl_player_types.h"

xl_statistics * xl_statistics_create(JNIEnv * jniEnv);
void xl_statistics_release(JNIEnv * jniEnv, xl_statistics * statistics);
void xl_statistics_reset(xl_statistics * statistics);
void xl_statistics_update(xl_statistics * statistics);
int xl_statistics_get_fps(xl_statistics * statistics);
int xl_statistics_get_bps(xl_statistics * statistics);

#endif //XL_XL_STATISTICS_H
