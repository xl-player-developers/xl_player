//
// Created by gutou on 2017/5/5.
//

#ifndef XL_XL_GLSL_PROGRAM_H
#define XL_XL_GLSL_PROGRAM_H

#include "../xl_types/xl_video_render_types.h"

xl_glsl_program * xl_glsl_program_get(ModelType type, int pixel_format);
xl_glsl_program_distortion * xl_glsl_program_distortion_get();
void xl_glsl_program_clear_all();

#endif //XL_XL_GLSL_PROGRAM_H
