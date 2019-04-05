//
// Created by gutou on 2017/5/24.
//

#ifndef XL_XL_VIDEO_RENDER_H
#define XL_XL_VIDEO_RENDER_H


#include "../xl_types/xl_player_types.h"
void xl_video_render_ctx_reset(xl_video_render_context * ctx);
xl_video_render_context * xl_video_render_ctx_create();
void xl_video_render_ctx_release(xl_video_render_context * ctx);

#endif //XL_XL_RENDER_H
