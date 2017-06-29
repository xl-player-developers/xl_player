//
// Created by gutou on 2017/5/4.
//

#ifndef XL_XL_TEXTURE_H
#define XL_XL_TEXTURE_H
#include "../xl_types/xl_player_types.h"
#include "../xl_types/xl_video_render_types.h"


void initTexture(xl_play_data * pd);
void xl_texture_delete(xl_play_data * pd);
void update_texture_yuv420p(xl_model *model, AVFrame *frame);
void update_texture_nv12(xl_model *model, AVFrame *frame);
void bind_texture_yuv420p(xl_model *model);
void bind_texture_nv12(xl_model *model);
void bind_texture_oes(xl_model *model);
void update_texture_oes(xl_model *model, AVFrame *frame);

#endif //XL_XL_TEXTURE_H
