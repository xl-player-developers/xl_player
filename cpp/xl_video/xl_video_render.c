//
// Created by gutou on 2017/5/24.
//

#include <pthread.h>
#include "xl_video_render.h"

void xl_video_render_set_window(xl_video_render_context *ctx, struct ANativeWindow *window) {
    pthread_mutex_lock(ctx->lock);
    ctx->window = window;
    ctx->cmd |= CMD_SET_WINDOW;
    pthread_mutex_unlock(ctx->lock);
}

void xl_video_render_change_model(xl_video_render_context *ctx, AAssetManager *pAAssetManager,
                                  ModelType model_type) {
    pthread_mutex_lock(ctx->lock);
    ctx->require_model_type = model_type;
    ctx->pAAssetManager = pAAssetManager;
    ctx->cmd |= CMD_CHANGE_MODEL;
    pthread_mutex_unlock(ctx->lock);
}

void xl_video_render_ctx_reset(xl_video_render_context * ctx){
    ctx->surface = EGL_NO_SURFACE;
    ctx->context = EGL_NO_CONTEXT;
    ctx->display = EGL_NO_DISPLAY;
    ctx->model = NULL;
    ctx->enable_tracker = false;
    ctx->require_model_type = NONE;
    ctx->require_model_scale = 1;
    ctx->cmd = NO_CMD;
    ctx->enable_tracker = false;
    ctx->draw_mode = wait_frame;
    ctx->require_model_rotation[0] = 0;
    ctx->require_model_rotation[1] = 0;
    ctx->require_model_rotation[2] = 0;
    ctx->width = ctx->height = 1;
}


xl_video_render_context *xl_video_render_ctx_create() {
    xl_video_render_context *ctx = malloc(sizeof(xl_video_render_context));
    ctx->lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ctx->lock, NULL);
    ctx->set_window = xl_video_render_set_window;
    ctx->change_model = xl_video_render_change_model;
    ctx->window = NULL;
    ctx->texture_window = NULL;
    xl_video_render_ctx_reset(ctx);
    return ctx;
}

void xl_video_render_ctx_release(xl_video_render_context *ctx) {
    pthread_mutex_destroy(ctx->lock);
    free(ctx->lock);
    free(ctx);
}