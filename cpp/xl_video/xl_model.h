//
// Created by gutou on 2017/4/11.
//

#ifndef WP_ANDROID_XL_MODEL_H
#define WP_ANDROID_XL_MODEL_H

#include "../xl_types/xl_video_render_types.h"



xl_model *createModel(ModelType mType);

void freeModel(xl_model *model);

#endif //WP_ANDROID_XL_MODEL_H
