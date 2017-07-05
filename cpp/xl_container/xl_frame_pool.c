//
// Created by gutou on 2017/5/8.
//

#include "xl_frame_pool.h"

static void get_frame_defaults(AVFrame *frame)
{
    // from ffmpeg libavutil frame.c
    if (frame->extended_data != frame->data)
        av_freep(&frame->extended_data);
    memset(frame, 0, sizeof(*frame));
    frame->pts                   =
    frame->pkt_dts               = AV_NOPTS_VALUE;
    frame->best_effort_timestamp = AV_NOPTS_VALUE;
    frame->pkt_duration        = 0;
    frame->pkt_pos             = -1;
    frame->pkt_size            = -1;
    frame->key_frame           = 1;
    frame->sample_aspect_ratio = (AVRational){ 0, 1 };
    frame->format              = -1; /* unknown */
    frame->extended_data       = frame->data;
    frame->color_primaries     = AVCOL_PRI_UNSPECIFIED;
    frame->color_trc           = AVCOL_TRC_UNSPECIFIED;
    frame->colorspace          = AVCOL_SPC_UNSPECIFIED;
    frame->color_range         = AVCOL_RANGE_UNSPECIFIED;
    frame->chroma_location     = AVCHROMA_LOC_UNSPECIFIED;
    frame->flags               = 0;

    frame->width = 0;
    frame->height = 0;
}

xl_frame_pool * xl_frame_pool_create(int size){
    xl_frame_pool * pool = (xl_frame_pool *)malloc(sizeof(xl_frame_pool));
    pool->size = size;
    pool->count = 0;
    pool->index = 0;
    pool->frames = (AVFrame *)av_mallocz(sizeof(AVFrame) * size);
    for(int i = 0; i < size; i++){
        get_frame_defaults(&pool->frames[i]);
    }
    return pool;
}

void xl_frame_pool_release(xl_frame_pool * pool){
    av_free(pool->frames);
    free(pool);
}

AVFrame * xl_frame_pool_get_frame(xl_frame_pool *pool){
    AVFrame * p = &pool->frames[pool->index];
    pool->index = (pool->index + 1) % pool->size;
    pool->count ++;
    return p;
}

void xl_frame_pool_unref_frame(xl_frame_pool * pool, AVFrame * frame){
    av_frame_unref(frame);
    pool->count--;
}