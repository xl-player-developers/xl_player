//
// Created by tianchi on 17-6-22.
//

#ifndef XL_XL_AUDIO_FILTER_H
#define XL_XL_AUDIO_FILTER_H

#include "../xl_types/xl_player_types.h"

xl_audio_filter_context * xl_audio_filter_create();
void xl_audio_filter_release(xl_audio_filter_context * ctx);
void xl_audio_filter_change_speed(xl_play_data * pd, float speed);
#endif //XL_XL_AUDIO_FILTER_H
