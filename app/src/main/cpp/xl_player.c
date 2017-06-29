//
// Created by gutou on 2017/4/18.
//
#include <jni.h>
#include <stdlib.h>
#include "xl_playerCore.h"
#include "xl_types/xl_macro.h"
#include "xl_video/xl_video_render.h"
#include "xl_utils/xl_statistics.h"

static xl_play_data *pd;
static AAssetManager *pAAssetManager;

static inline void changeModel(ModelType modelType) {
    if (pd->video_render_ctx->require_model_type != modelType) {
        pd->video_render_ctx->change_model(pd->video_render_ctx, pAAssetManager, modelType);
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_initPlayer(JNIEnv *env, jobject instance,
                                                              jobject xlPlayer, jobject manager,
                                                              jint runAndroidVersion,jint bestSampleRate) {
    pd = xl_player_create(env, xlPlayer, runAndroidVersion,bestSampleRate);
    if (env && manager) {
        pAAssetManager = AAssetManager_fromJava(env, manager);
    } else {
        LOGE("get AssetManager error!");
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setForceSwDecode(JNIEnv *env, jobject instance,
                                                                    jboolean forceSwDecode) {
    if (pd != NULL) {
        pd->force_sw_decode = forceSwDecode;
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setSurface(JNIEnv *env, jobject instance,
                                                              jobject surface) {
    if (pd != NULL) {
        if (pd->video_render_ctx->window != NULL) {
            ANativeWindow_release(pd->video_render_ctx->window);
        }
        ANativeWindow *sur = ANativeWindow_fromSurface(env, surface);
        pd->video_render_ctx->set_window(pd->video_render_ctx, sur);
    }
}

JNIEXPORT int JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_play(JNIEnv *env, jobject instance, jstring url_,
                                                        jfloat time, jint model) {
    if (pd == NULL || pd->status != IDEL) {
        return 0;
    }
    const char *url = (*env)->GetStringUTFChars(env, url_, 0);
    changeModel((ModelType) model);
    int ret = xl_player_play(url, time, pd);
    (*env)->ReleaseStringUTFChars(env, url_, url);
    return ret;
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_changeModel(JNIEnv *env, jobject instance,
                                                               jint model) {
    if (pd != NULL && pd->status == PLAYING) {
        changeModel((ModelType) model);
    }
}

float getCurrentTime() {
    if (pd) {
        if (pd->av_track_flags & XL_HAS_AUDIO_FLAG) {
            return (float) pd->audio_clock->pts / 1000000;
        } else if (pd->av_track_flags & XL_HAS_VIDEO_FLAG) {
            return (float) pd->video_clock->pts / 1000000;
        }
    }
    return 0.0f;
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_seek(JNIEnv *env, jobject instance, jfloat time,
                                                        jboolean isSeekTo) {
    if(pd && pd->status != IDEL) {
        if (isSeekTo) {
            xl_player_seek(pd, time);
        } else {
            xl_player_seek(pd, time + getCurrentTime());
        }
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_pause(JNIEnv *env, jobject instance) {
    if (pd && pd->status == PLAYING) {
        pd->change_status(pd, PAUSED);
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_resume(JNIEnv *env, jobject instance) {
    if (pd && pd->status == PAUSED) {
        xl_player_resume(pd);
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_stop(JNIEnv *env, jobject instance) {
    xl_player_stop(pd);
}

JNIEXPORT jfloat JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_getTotalTime(JNIEnv *env, jobject instance) {
    if (pd && pd->status == PLAYING) {
        if (pd->pFormatCtx->duration != AV_NOPTS_VALUE) {
            return (float) pd->pFormatCtx->duration / AV_TIME_BASE;
        }
    }
    return 0.0f;
}

JNIEXPORT jfloat JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_getCurrentTime(JNIEnv *env, jobject instance) {
    if (pd->seeking > 0) {
        return pd->seek_to;
    } else {
        return getCurrentTime();
    }

}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_release(JNIEnv *env, jobject instance) {
    xl_player_release(pd);
    pd = NULL;
}

JNIEXPORT jint JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_getPlayStatus(JNIEnv *env, jobject instance) {
    return pd == NULL ? UNINIT : pd->status;
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_rotate(JNIEnv *env, jobject instance,
                                                          jboolean clockwise) {

    if (pd != NULL) {
        if (clockwise) {
            pd->frame_rotation++;
        } else {
            pd->frame_rotation--;
        }
        if (pd->frame_rotation > XL_ROTATION_270) {
            pd->frame_rotation = XL_ROTATION_0;
        }
        if (pd->frame_rotation < XL_ROTATION_0) {
            pd->frame_rotation = XL_ROTATION_270;
        }
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_resize(JNIEnv *env, jobject instance, jint width,
                                                          jint height) {
    if (pd != NULL && pd->video_render_ctx != NULL) {

        if (pd->video_render_ctx->model != NULL) {
            xl_model *model = pd->video_render_ctx->model;
            model->resize(model, width, height);
        }
        pd->video_render_ctx->width = width;
        pd->video_render_ctx->height = height;
    }

}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_changeRate(JNIEnv *env, jobject instance,
                                                              jfloat rate) {
    if (pd != NULL) {
        if (rate < 0.5 || rate > 2.0) {
            return;
        }
//        pd->audio_ctx->changeSpeed((short) (1000 * rate));
        change_audio_speed(rate, pd);
    }

}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setScale(JNIEnv *env, jobject instance,
                                                            jfloat scale) {

    if (pd != NULL && pd->video_render_ctx != NULL) {
        pd->video_render_ctx->require_model_scale = scale;
        pd->video_render_ctx->cmd |= CMD_CHANGE_SCALE;
    }

}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setRotation(JNIEnv *env, jobject instance,
                                                               jfloat rx, jfloat ry, jfloat rz) {
    if (pd != NULL && pd->video_render_ctx != NULL) {
        pd->video_render_ctx->require_model_rotation[0] = rx;
        pd->video_render_ctx->require_model_rotation[1] = ry;
        pd->video_render_ctx->require_model_rotation[2] = rz;
        pd->video_render_ctx->cmd |= CMD_CHANGE_ROTATION;
    }
}


JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setHeadTrackerEnable(JNIEnv *env,
                                                                        jobject instance,
                                                                        jboolean enable) {
    if (pd != NULL && pd->video_render_ctx != NULL) {
        pd->video_render_ctx->enable_tracker = enable;
    }

}

JNIEXPORT jboolean JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_getHeadTrackerEnable(JNIEnv *env,
                                                                        jobject instance) {

    if (pd != NULL && pd->video_render_ctx != NULL) {
        return (jboolean) pd->video_render_ctx->enable_tracker;
    }
    return false;
}

static void write_int_to_buffer(uint8_t * buf, int val){
    buf[0] = (uint8_t) ((val >> 24) & 0xff);
    buf[1] = (uint8_t) ((val >> 16) & 0xff);
    buf[2] = (uint8_t) ((val >> 8) & 0xff);
    buf[3] = (uint8_t) (val & 0xff);
}
JNIEXPORT jobject JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_getStatistics(JNIEnv *env, jclass type) {
    if (pd != NULL && pd->statistics != NULL) {
        if(pd->statistics->last_update_time > 0){
            int fps = xl_statistics_get_fps(pd->statistics);
            int bps = xl_statistics_get_bps(pd->statistics);
            int buffer_time = 0;
            AVRational time_base;
            if(pd->av_track_flags & XL_HAS_VIDEO_FLAG){
                time_base = pd->pFormatCtx->streams[pd->videoIndex]->time_base;
                buffer_time = (int)((double)pd->video_packet_queue->duration * 1000 * av_q2d(time_base));
            }else{
                time_base = pd->pFormatCtx->streams[pd->audioIndex]->time_base;
                buffer_time = (int)((double)pd->audio_packet_queue->duration * 1000 * av_q2d(time_base));
            }
            write_int_to_buffer(pd->statistics->ret_buffer, fps);
            write_int_to_buffer(pd->statistics->ret_buffer + 4, bps);
            write_int_to_buffer(pd->statistics->ret_buffer + 8, buffer_time);
        }
        xl_statistics_update(pd->statistics);
        return pd->statistics->ret_buffer_java;
    }
    return NULL;
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setBufferTime(JNIEnv *env, jclass type,
                                                                 jfloat time) {
    if(pd){
        xl_player_set_buffer_time(pd, time);
    }
}

JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_setPlayBackground(JNIEnv *env, jclass type,
                                                                     jboolean playBackground) {
    if(pd){
        xl_player_set_play_background(pd, playBackground);
    }
}