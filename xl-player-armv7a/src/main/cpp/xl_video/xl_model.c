//
// Created by gutou on 2017/4/11.
//

#include "xl_model.h"

#include "xl_model_rect.h"
#include "xl_model_ball.h"
#include "xl_model_vr.h"


xl_model *createModel(ModelType mType) {
    switch (mType) {
        case Rect:
            return model_rect_create();
        case Ball:
            return model_ball_create();
        case VR:
            return model_vr_create();
        case Planet:
            return model_planet_create();
        case Architecture:
            return model_architecture_create();
        case Expand:
            return model_expand_create();
        default:
            LOGE("invalid model type");
           return NULL;
    }
}

void freeModel(xl_model *model) {
    if(model != NULL){
        glDeleteBuffers(3, model->vbos);
        if(model->type == VR){
            model_vr_free(model);
        }
        free(model);
    }
}
