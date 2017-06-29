//
// Created by gutou on 2017/4/20.
//

#ifndef XL_XL_AUDIO_PLAYER_H
#define XL_XL_AUDIO_PLAYER_H

#include "../xl_types/xl_player_types.h"


xl_audio_player_context *  xl_audio_engine_create();
void xl_audio_player_create(int rate, int channel, xl_play_data * pd);

#endif //XL_XL_AUDIO_PLAYER_H
