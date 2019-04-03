//
// Created by gutou on 2017/5/8.
//

#ifndef XL_XL_FRAME_POOL_H
#define XL_XL_FRAME_POOL_H
#include "../xl_types/xl_player_types.h"
xl_frame_pool * xl_frame_pool_create(int size);

void xl_frame_pool_release(xl_frame_pool * pool);

AVFrame * xl_frame_pool_get_frame(xl_frame_pool *pool);

void xl_frame_pool_unref_frame(xl_frame_pool * pool, AVFrame * frame);
#endif //XL_XL_FRAME_POOL_H
