//
// Created by gutou on 2017/4/18.
//

#ifndef XL_XL_PLAYERCORE_H
#define XL_XL_PLAYERCORE_H

#include <pthread.h>
#include "xl_container/xl_packet_pool.h"
#include "xl_container/xl_packet_queue.h"
#include "xl_utils/xl_clock.h"
#include "xl_decoders/xl_mediacodec.h"

xl_play_data *  xl_player_create(JNIEnv *env, jobject instance,int xl_player_create,int best_samplerate);
void xl_player_set_buffer_time(xl_play_data *pd, float buffer_time);
int xl_player_play(const char * url, float timee, xl_play_data *pd);
int xl_player_resume(xl_play_data * pd);
void xl_player_seek(xl_play_data * pd, float seek_to);
void xl_player_set_play_background(xl_play_data * pd, bool play_background);
int xl_player_stop(xl_play_data *pd);
int xl_player_release(xl_play_data *pd);
void change_audio_speed(float speede, xl_play_data *pd);
void change_status(xl_play_data *pd, PlayStatus statuse);
#endif //XL_XL_PLAYERCORE_H
