//
// Created by gutou on 2017/4/18.
//

#include "xl_playerCore.h"
#include <unistd.h>
#include "xl_audio/xl_audio_player.h"
#include "xl_player_read_thread.h"
#include "xl_container/xl_frame_pool.h"
#include "xl_container/xl_frame_queue.h"
#include "xl_decoders/xl_player_video_sw_decode_thread.h"
#include "xl_decoders/xl_player_video_hw_decode_thread.h"
#include "xl_decoders/xl_player_audio_decode_thread.h"
#include "xl_video/xl_video_render.h"
#include "xl_video/xl_player_gl_thread.h"
#include "xl_utils/xl_jni_reflect.h"
#include "xl_audio/xl_audio_filter.h"
#include "xl_utils/xl_statistics.h"

static int stop(xl_play_data *pd);

static void send_message(xl_play_data *pd, int message) {
    int sig = message;
    write(pd->pipe_fd[1], &sig, sizeof(int));
}

static int message_callback(int fd, int events, void *data) {
    xl_play_data *pd = data;
    int message;
    for (int i = 0; i < events; i++) {
        read(fd, &message, sizeof(int));
        LOGI("recieve message ==> %d", message);
        switch (message) {
            case xl_message_stop:
                stop(pd);
                break;
            case xl_message_buffer_empty:
                change_status(pd, BUFFER_EMPTY);
                break;
            case xl_message_buffer_full:
                change_status(pd, BUFFER_FULL);
                break;
            default:
                break;
        }
    }
    return 1;
}

static void buffer_empty_cb(void *data) {
    xl_play_data *pd = data;
    if (pd->status != BUFFER_EMPTY && !pd->eof) {
        pd->send_message(pd, xl_message_buffer_empty);
    }
}

static void buffer_full_cb(void *data) {
    xl_play_data *pd = data;
    if (pd->status == BUFFER_EMPTY) {
        pd->send_message(pd, xl_message_buffer_full);
    }
}


static void reset(xl_play_data *pd) {
    if (pd == NULL) return;
    pd->eof = false;
    pd->av_track_flags = 0;
    pd->just_audio = false;
    pd->videoIndex = -1;
    pd->audioIndex = -1;
    pd->width = 0;
    pd->height = 0;
    pd->audio_frame = NULL;
    pd->video_frame = NULL;
    pd->audioBufferSize = 0;
    pd->audioBuffer = NULL;
    pd->seeking = 0;
    xl_clock_reset(pd->audio_clock);
    xl_clock_reset(pd->video_clock);
    xl_statistics_reset(pd->statistics);
    pd->error_code = 0;
    pd->buffer_time_length = 5.0f;
    pd->frame_rotation = XL_ROTATION_0;
    xl_packet_pool_reset(pd->packet_pool);
    pd->change_status(pd, IDEL);
    xl_video_render_ctx_reset(pd->video_render_ctx);
}

static inline void set_buffer_time(xl_play_data *pd) {
    float buffer_time_length = pd->buffer_time_length;
    AVRational time_base;
    if (pd->av_track_flags & XL_HAS_AUDIO_FLAG) {
        time_base = pd->pFormatCtx->streams[pd->audioIndex]->time_base;
        xl_queue_set_duration(pd->audio_packet_queue,
                              (uint64_t) (buffer_time_length / av_q2d(time_base)));
        pd->audio_packet_queue->empty_cb = buffer_empty_cb;
        pd->audio_packet_queue->full_cb = buffer_full_cb;
        pd->audio_packet_queue->cb_data = pd;
    } else if (pd->av_track_flags & XL_HAS_VIDEO_FLAG) {
        time_base = pd->pFormatCtx->streams[pd->videoIndex]->time_base;
        xl_queue_set_duration(pd->video_packet_queue,
                              (uint64_t) (buffer_time_length / av_q2d(time_base)));
        pd->video_packet_queue->empty_cb = buffer_empty_cb;
        pd->video_packet_queue->full_cb = buffer_full_cb;
        pd->video_packet_queue->cb_data = pd;
    }

}

xl_play_data *
xl_player_create(JNIEnv *env, jobject instance, int run_android_version, int best_samplerate) {
    xl_play_data *pd = (xl_play_data *) malloc(sizeof(xl_play_data));
    pd->jniEnv = env;
    (*env)->GetJavaVM(env, &pd->vm);
    pd->xlPlayer = (*pd->jniEnv)->NewGlobalRef(pd->jniEnv, instance);
    xl_jni_reflect_java_class(&pd->jc, pd->jniEnv);
    pd->run_android_version = run_android_version;
    pd->best_samplerate = best_samplerate;
    pd->audio_packet_queue = xl_queue_create(100);
    pd->video_packet_queue = xl_queue_create(100);
    pd->video_frame_pool = xl_frame_pool_create(6);
    pd->video_frame_queue = xl_frame_queue_create(4);
    pd->audio_frame_pool = xl_frame_pool_create(12);
    pd->audio_frame_queue = xl_frame_queue_create(10);
    pd->packet_pool = xl_packet_pool_create(400);
    pd->audio_clock = xl_clock_creat();
    pd->video_clock = xl_clock_creat();
    pd->statistics = xl_statistics_create(pd->jniEnv);
    av_register_all();
    avfilter_register_all();
    avformat_network_init();
    pd->audio_ctx = xl_audio_engine_create();
    pd->audio_filter_ctx = xl_audio_filter_create();
    pd->video_render_ctx = xl_video_render_ctx_create();
    pd->main_looper = ALooper_forThread();
    pipe(pd->pipe_fd);
    if (1 !=
        ALooper_addFd(pd->main_looper, pd->pipe_fd[0], ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
                      message_callback, pd)) {
        LOGE("error. when add fd to main looper");
    }
    pd->change_status = change_status;
    pd->send_message = send_message;
    reset(pd);
    return pd;
}

static int audio_codec_init(xl_play_data *pd) {
    pd->pAudioCodec = avcodec_find_decoder(
            pd->pFormatCtx->streams[pd->audioIndex]->codecpar->codec_id);
    if (pd->pAudioCodec == NULL) {
        LOGE("could not find audio codec\n");
        return 202;
    }
    pd->pAudioCodecCtx = avcodec_alloc_context3(pd->pAudioCodec);
    avcodec_parameters_to_context(pd->pAudioCodecCtx,
                                  pd->pFormatCtx->streams[pd->audioIndex]->codecpar);
    if (avcodec_open2(pd->pAudioCodecCtx, pd->pAudioCodec, NULL) < 0) {
        avcodec_free_context(&pd->pAudioCodecCtx);
        LOGE("could not open audio codec\n");
        return 203;
    }
    // Android openSL ES   can not support more than 2 channels.
    int channels = pd->pAudioCodecCtx->channels <= 2 ? pd->pAudioCodecCtx->channels : 2;
    pd->audio_ctx->player_create(pd->best_samplerate, channels, pd);
    xl_audio_filter_change_speed(pd, 1.0);
    return 0;
}

static int sw_codec_init(xl_play_data *pd) {
    pd->pVideoCodec = avcodec_find_decoder(
            pd->pFormatCtx->streams[pd->videoIndex]->codecpar->codec_id);
    if (pd->pVideoCodec == NULL) {
        LOGE("could not find video codec\n");
        return 102;
    }
    pd->pVideoCodecCtx = avcodec_alloc_context3(pd->pVideoCodec);
    avcodec_parameters_to_context(pd->pVideoCodecCtx,
                                  pd->pFormatCtx->streams[pd->videoIndex]->codecpar);
    AVDictionary *options = NULL;
    av_dict_set(&options, "threads", "auto", 0);
    av_dict_set(&options, "refcounted_frames", "1", 0);
    if (avcodec_open2(pd->pVideoCodecCtx, pd->pVideoCodec, &options) < 0) {
        avcodec_free_context(&pd->pVideoCodecCtx);
        LOGE("could not open video codec\n");
        return 103;
    }
    return 0;
}

static int hw_codec_init(xl_play_data *pd) {
    pd->pMediaCodecCtx = xl_create_mediacodec_context(pd);
    return 0;
}

int xl_player_play(const char *url, float time, xl_play_data *pd) {
    int i, ret;
    pd->pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pd->pFormatCtx, url, NULL, NULL) != 0) {
        LOGE("can not open url\n");
        ret = 100;
        goto fail;
    }
    if (avformat_find_stream_info(pd->pFormatCtx, NULL) < 0) {
        LOGE("can not find stream\n");
        ret = 101;
        goto fail;
    }
    i = av_find_best_stream(pd->pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (i != AVERROR_STREAM_NOT_FOUND) {
        pd->videoIndex = i;
        pd->av_track_flags |= XL_HAS_VIDEO_FLAG;
    }
    i = av_find_best_stream(pd->pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (i != AVERROR_STREAM_NOT_FOUND) {
        pd->audioIndex = i;
        pd->av_track_flags |= XL_HAS_AUDIO_FLAG;
    }
    AVCodecParameters *codecpar;

    float buffer_time_length = pd->buffer_time_length;
    if (pd->av_track_flags & XL_HAS_AUDIO_FLAG) {
        ret = audio_codec_init(pd);
        if (ret != 0) {
            goto fail;
        }
    }

    if (pd->av_track_flags & XL_HAS_VIDEO_FLAG) {
        codecpar = pd->pFormatCtx->streams[pd->videoIndex]->codecpar;
        pd->width = codecpar->width;
        pd->height = codecpar->height;
        if (pd->force_sw_decode) {
            pd->is_sw_decode = true;
        } else {
            switch (codecpar->codec_id) {
                case AV_CODEC_ID_H264:
                case AV_CODEC_ID_HEVC:
                case AV_CODEC_ID_MPEG4:
                case AV_CODEC_ID_VP8:
                case AV_CODEC_ID_VP9:
                case AV_CODEC_ID_H263:
                    pd->is_sw_decode = false;
                    break;
                default:
                    pd->is_sw_decode = true;
                    break;
            }
        }
        if (pd->is_sw_decode) {
            ret = sw_codec_init(pd);
        } else {
            ret = hw_codec_init(pd);
        }
        if (ret != 0) {
            goto fail;
        }
        AVStream *v_stream = pd->pFormatCtx->streams[pd->videoIndex];
        AVDictionaryEntry *m = NULL;
        m = av_dict_get(v_stream->metadata, "rotate", m, AV_DICT_MATCH_CASE);
        if (m != NULL) {
            switch (atoi(m->value)) {
                case 90:
                    pd->frame_rotation = XL_ROTATION_90;
                    break;
                case 180:
                    pd->frame_rotation = XL_ROTATION_180;
                    break;
                case 270:
                    pd->frame_rotation = XL_ROTATION_270;
                    break;
                default:
                    break;
            }
        }
    }
    set_buffer_time(pd);
    if (time > 0) {
        xl_player_seek(pd, time);
    }
    pthread_create(&pd->read_stream_thread, NULL, read_thread, pd);
    if (pd->av_track_flags & XL_HAS_VIDEO_FLAG) {
        if (pd->is_sw_decode) {
            pthread_create(&pd->video_decode_thread, NULL, video_decode_sw_thread, pd);
        } else {
            pthread_create(&pd->video_decode_thread, NULL, video_decode_hw_thread, pd);
        }
        pthread_create(&pd->gl_thread, NULL, xl_player_gl_thread, pd);
    }
    if (pd->av_track_flags & XL_HAS_AUDIO_FLAG) {
        pthread_create(&pd->audio_decode_thread, NULL, audio_decode_thread, pd);
    }
    pd->change_status(pd, PLAYING);
    return ret;

    fail:
    if (pd->pFormatCtx) {
        avformat_close_input(&pd->pFormatCtx);
    }
    return ret;
}

void xl_player_set_buffer_time(xl_play_data *pd, float buffer_time) {
    pd->buffer_time_length = buffer_time;
    if (pd->status != IDEL) {
        set_buffer_time(pd);
    }
}

int xl_player_resume(xl_play_data *pd) {
    pd->change_status(pd, PLAYING);
    pd->audio_ctx->play(pd);
    return 0;
}

void xl_player_seek(xl_play_data *pd, float seek_to) {
    float total_time = (float) pd->pFormatCtx->duration / AV_TIME_BASE;
    seek_to = seek_to >= 0 ? seek_to : 0;
    seek_to = seek_to <= total_time ? seek_to : total_time;
    pd->seek_to = seek_to;
    pd->seeking = 1;
}

void xl_player_set_play_background(xl_play_data *pd, bool play_background) {
    if (pd->just_audio) {
        if (pd->is_sw_decode) {
            avcodec_flush_buffers(pd->pVideoCodecCtx);
        } else {
            xl_mediacodec_flush(pd);
        }
    }
    pd->just_audio = play_background;
}

static inline void clean_queues(xl_play_data *pd) {
    AVPacket *packet;
    // clear pd->audio_frame audio_frame_queue  audio_packet_queue
    if ((pd->av_track_flags & XL_HAS_AUDIO_FLAG) > 0) {
        if (pd->audio_frame != NULL) {
            if (pd->audio_frame != &pd->audio_frame_queue->flush_frame) {
                xl_frame_pool_unref_frame(pd->audio_frame_pool, pd->audio_frame);
            }
        }
        while (1) {
            pd->audio_frame = xl_frame_queue_get(pd->audio_frame_queue);
            if (pd->audio_frame == NULL) {
                break;
            }
            if (pd->audio_frame != &pd->audio_frame_queue->flush_frame) {
                xl_frame_pool_unref_frame(pd->audio_frame_pool, pd->audio_frame);
            }
        }
        while (1) {
            packet = xl_packet_queue_get(pd->audio_packet_queue);
            if (packet == NULL) {
                break;
            }
            if (packet != &pd->audio_packet_queue->flush_packet) {
                xl_packet_pool_unref_packet(pd->packet_pool, packet);
            }
        }
    }
    // clear pd->video_frame video_frame_queue video_frame_packet
    if ((pd->av_track_flags & XL_HAS_VIDEO_FLAG) > 0) {
        if (pd->video_frame != NULL) {
            if (pd->video_frame != &pd->video_frame_queue->flush_frame) {
                xl_frame_pool_unref_frame(pd->video_frame_pool, pd->video_frame);
            }
        }
        while (1) {
            pd->video_frame = xl_frame_queue_get(pd->video_frame_queue);
            if (pd->video_frame == NULL) {
                break;
            }
            if (pd->video_frame != &pd->video_frame_queue->flush_frame) {
                xl_frame_pool_unref_frame(pd->video_frame_pool, pd->video_frame);
            }
        }
        while (1) {
            packet = xl_packet_queue_get(pd->video_packet_queue);
            if (packet == NULL) {
                break;
            }
            if (packet != &pd->video_packet_queue->flush_packet) {
                xl_packet_pool_unref_packet(pd->packet_pool, packet);
            }
        }
    }
}

static int stop(xl_play_data *pd) {
    // remove buffer call back
    pd->audio_packet_queue->empty_cb = NULL;
    pd->audio_packet_queue->full_cb = NULL;
    pd->video_packet_queue->empty_cb = NULL;
    pd->video_packet_queue->full_cb = NULL;

    clean_queues(pd);
    // 停止各个thread
    void *thread_res;
    pthread_join(pd->read_stream_thread, &thread_res);
    if ((pd->av_track_flags & XL_HAS_VIDEO_FLAG) > 0) {
        pthread_join(pd->video_decode_thread, &thread_res);
        if (pd->is_sw_decode) {
            avcodec_free_context(&pd->pVideoCodecCtx);
        } else {
            xl_mediacodec_release_context(pd);
        }
        pthread_join(pd->gl_thread, &thread_res);
    }

    if ((pd->av_track_flags & XL_HAS_AUDIO_FLAG) > 0) {
        pthread_join(pd->audio_decode_thread, &thread_res);
        avcodec_free_context(&pd->pAudioCodecCtx);
        pd->audio_ctx->shutdown();
    }

    clean_queues(pd);

    free(pd->audioBuffer);
    avformat_close_input(&pd->pFormatCtx);
    reset(pd);
    LOGI("player stoped");
    return 0;
}

int xl_player_stop(xl_play_data *pd) {
    if (pd == NULL || pd->status == IDEL) return 0;
    pd->error_code = -1;
    return stop(pd);
}

int xl_player_release(xl_play_data *pd) {
    if (pd->status != IDEL) {
        xl_player_stop(pd);
    }
    while (pd->status != IDEL) {
        usleep(10000);
    }
    ALooper_removeFd(pd->main_looper, pd->pipe_fd[0]);
    close(pd->pipe_fd[1]);
    close(pd->pipe_fd[0]);
    xl_frame_queue_free(pd->audio_frame_queue);
    xl_frame_queue_free(pd->video_frame_queue);
    xl_frame_pool_release(pd->audio_frame_pool);
    xl_frame_pool_release(pd->video_frame_pool);
    xl_packet_queue_free(pd->audio_packet_queue);
    xl_packet_queue_free(pd->video_packet_queue);
    xl_packet_pool_release(pd->packet_pool);
    xl_statistics_release(pd->jniEnv, pd->statistics);
    xl_clock_free(pd->audio_clock);
    xl_clock_free(pd->video_clock);
    xl_video_render_ctx_release(pd->video_render_ctx);
    pd->audio_ctx->release();
    xl_audio_filter_release(pd->audio_filter_ctx);
    (*pd->jniEnv)->DeleteGlobalRef(pd->jniEnv, pd->xlPlayer);
    xl_jni_free(&pd->jc, pd->jniEnv);
    free(pd);
    return 0;
}

void change_status(xl_play_data *pd, PlayStatus status) {
    if (status == BUFFER_FULL) {
        xl_player_resume(pd);
    } else {
        pd->status = status;
    }
    (*pd->jniEnv)->CallVoidMethod(pd->jniEnv, pd->xlPlayer, pd->jc->player_onPlayStatusChanged,
                                  status);
}

void change_audio_speed(float speed, xl_play_data *pd) {
    xl_audio_filter_change_speed(pd, speed);
}