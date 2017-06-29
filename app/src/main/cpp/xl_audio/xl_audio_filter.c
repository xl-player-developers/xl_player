//
// Created by tianchi on 17-6-22.
//

#include <pthread.h>
#include <libavutil/opt.h>
#include "xl_audio_filter.h"

xl_audio_filter_context *xl_audio_filter_create() {
    xl_audio_filter_context *ctx = malloc(sizeof(xl_audio_filter_context));
    ctx->filter_lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ctx->filter_lock, NULL);
    ctx->filter_graph = NULL;
    ctx->buffersrc_ctx = NULL;
    ctx->buffersink_ctx = NULL;
    ctx->abuffersrc = avfilter_get_by_name("abuffer");
    ctx->abuffersink = avfilter_get_by_name("abuffersink");
    return ctx;
}

void xl_audio_filter_release(xl_audio_filter_context *ctx) {
    if (ctx->buffersrc_ctx != NULL) {
        avfilter_free(ctx->buffersrc_ctx);
    }
    if (ctx->buffersink_ctx != NULL) {
        avfilter_free(ctx->buffersink_ctx);
    }
    if (ctx->filter_graph != NULL) {
        avfilter_graph_free(&ctx->filter_graph);
    }
    pthread_mutex_destroy(ctx->filter_lock);
    free(ctx->filter_lock);
    free(ctx);
}

void xl_audio_filter_change_speed(xl_play_data *pd, float speed) {
    xl_audio_filter_context *ctx = pd->audio_filter_ctx;

    pthread_mutex_lock(ctx->filter_lock);
    if (ctx->buffersrc_ctx != NULL) {
        avfilter_free(ctx->buffersrc_ctx);
    }
    if (ctx->buffersink_ctx != NULL) {
        avfilter_free(ctx->buffersink_ctx);
    }
    if (ctx->filter_graph != NULL) {
        avfilter_graph_free(&ctx->filter_graph);
    }
    ctx->inputs = avfilter_inout_alloc();
    ctx->outputs = avfilter_inout_alloc();
    ctx->filter_graph = avfilter_graph_alloc();
    AVRational time_base = pd->pFormatCtx->streams[pd->audioIndex]->time_base;
    AVCodecContext *dec_ctx = pd->pAudioCodecCtx;
    enum AVSampleFormat out_sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};
    int out_sample_rates[] = {0, -1};
    out_sample_rates[0] = pd->best_samplerate;
    int out_sample_channel_count[] = {1, -1};
    int channels = dec_ctx->channels <= 2 ? dec_ctx->channels : 2;
    out_sample_channel_count[0] = channels;
    int64_t out_channel_layouts[] = {AV_CH_LAYOUT_MONO, -1};
    if (channels == 2) {
        out_channel_layouts[0] = AV_CH_LAYOUT_STEREO;
    }


    char args[512];
    snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channels=%d:channel_layout=0x%"PRIx64,
             time_base.num, time_base.den, dec_ctx->sample_rate,
             av_get_sample_fmt_name(dec_ctx->sample_fmt), dec_ctx->channels,
             dec_ctx->channel_layout);
    avfilter_graph_create_filter(&ctx->buffersrc_ctx, ctx->abuffersrc, "in", args, NULL,
                                 ctx->filter_graph);
    avfilter_graph_create_filter(&ctx->buffersink_ctx, ctx->abuffersink, "out", NULL, NULL,
                                 ctx->filter_graph);

    av_opt_set_int_list(ctx->buffersink_ctx, "sample_fmts", out_sample_fmts, -1,
                        AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int_list(ctx->buffersink_ctx, "channel_layouts", out_channel_layouts, -1,
                        AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int_list(ctx->buffersink_ctx, "sample_rates", out_sample_rates, -1,
                        AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int_list(ctx->buffersink_ctx, "channel_counts", out_sample_channel_count, -1,
                        AV_OPT_SEARCH_CHILDREN);

    ctx->outputs->name = av_strdup("in");
    ctx->outputs->filter_ctx = ctx->buffersrc_ctx;
    ctx->outputs->pad_idx = 0;
    ctx->outputs->next = NULL;

    ctx->inputs->name = av_strdup("out");
    ctx->inputs->filter_ctx = ctx->buffersink_ctx;
    ctx->inputs->pad_idx = 0;
    ctx->inputs->next = NULL;

    char filter_descr[128] = {0};
    sprintf(filter_descr, "atempo=%.2f", speed);
    avfilter_graph_parse_ptr(ctx->filter_graph, filter_descr, &ctx->inputs, &ctx->outputs, NULL);
    avfilter_graph_config(ctx->filter_graph, NULL);
//    ctx->outlink = ctx->buffersink_ctx->inputs[0];
    pthread_mutex_unlock(ctx->filter_lock);
    avfilter_inout_free(&ctx->inputs);
    avfilter_inout_free(&ctx->outputs);
}