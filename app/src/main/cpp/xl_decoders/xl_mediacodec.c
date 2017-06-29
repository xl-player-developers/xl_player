//
// Created by gutou on 2017/4/24.
//

#include <unistd.h>
#include "xl_mediacodec.h"

#if __ANDROID_API__ >= NDK_MEDIACODEC_VERSION

int xl_mediacodec_send_packet(xl_play_data *pd, AVPacket *packet) {
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    if (packet == NULL) { return -2; }
    uint32_t keyframe_flag = 0;
//    av_packet_split_side_data(packet);
    int64_t time_stamp = packet->pts;
    if (!time_stamp && packet->dts)
        time_stamp = packet->dts;
    if (time_stamp > 0) {
        time_stamp = av_rescale_q(time_stamp, pd->pFormatCtx->streams[pd->videoIndex]->time_base,
                                  AV_TIME_BASE_Q);
    } else {
        time_stamp = 0;
    }
    if (ctx->codec_id == AV_CODEC_ID_H264 ||
        ctx->codec_id == AV_CODEC_ID_HEVC) {
        H264ConvertState convert_state = {0, 0};
        convert_h264_to_annexb(packet->data, (size_t) packet->size, ctx->nal_size, &convert_state);
    }
    if ((packet->flags | AV_PKT_FLAG_KEY) > 0) {
        keyframe_flag |= 0x1;
    }
    ssize_t id = AMediaCodec_dequeueInputBuffer(ctx->codec, 1000000);
    media_status_t media_status;
    size_t size;
    if (id >= 0) {
        uint8_t *buf = AMediaCodec_getInputBuffer(ctx->codec, (size_t) id, &size);
        if (buf != NULL && size >= packet->size) {
            memcpy(buf, packet->data, (size_t) packet->size);
            media_status = AMediaCodec_queueInputBuffer(ctx->codec, (size_t) id, 0, (size_t) packet->size,
                                                        (uint64_t) time_stamp,
                                                        keyframe_flag);
            if (media_status != AMEDIA_OK) {
                LOGE("AMediaCodec_queueInputBuffer error. status ==> %d", media_status);
                return (int) media_status;
            }
        }
    }else if(id == AMEDIACODEC_INFO_TRY_AGAIN_LATER){
        return -1;
    }else{
        LOGE("input buffer id < 0  value == %zd", id);
    }
    return 0;
}

void xl_mediacodec_release_buffer(xl_play_data *pd, AVFrame *frame) {
    AMediaCodec_releaseOutputBuffer(pd->pMediaCodecCtx->codec, (size_t) frame->HW_BUFFER_ID, true);
}

int xl_mediacodec_receive_frame(xl_play_data *pd, AVFrame *frame) {
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    AMediaCodecBufferInfo info;
    int output_ret = 1;
    ssize_t outbufidx = AMediaCodec_dequeueOutputBuffer(ctx->codec, &info, 0);
    if (outbufidx >= 0) {
            frame->pts = info.presentationTimeUs;
            frame->format = XL_PIX_FMT_EGL_EXT;
            frame->width = pd->width;
            frame->linesize[0] = pd->width;
            frame->height = pd->height;
            frame->HW_BUFFER_ID = outbufidx;
            output_ret = 0;
    } else {
        switch (outbufidx) {
            case AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED: {
                int pix_format = -1;
                AMediaFormat *format = AMediaCodec_getOutputFormat(ctx->codec);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &ctx->width);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &ctx->height);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &pix_format);
                //todo 仅支持了两种格式
                switch (pix_format) {
                    case 19:
                        ctx->pix_format = AV_PIX_FMT_YUV420P;
                        break;
                    case 21:
                        ctx->pix_format = AV_PIX_FMT_NV12;
                        break;
                    default:
                        break;
                }
                output_ret = -2;
                break;
            }
            case AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED:
                break;
            case AMEDIACODEC_INFO_TRY_AGAIN_LATER:
                break;
            default:
                break;
        }

    }
    return output_ret;
}

void xl_mediacodec_flush(xl_play_data *pd) {
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    AMediaCodec_flush(ctx->codec);
}

xl_mediacodec_context *xl_create_mediacodec_context(xl_play_data *pd) {
    xl_mediacodec_context *ctx = (xl_mediacodec_context *) malloc(sizeof(xl_mediacodec_context));
    AVCodecParameters *codecpar = pd->pFormatCtx->streams[pd->videoIndex]->codecpar;
    ctx->width = codecpar->width;
    ctx->height = codecpar->height;
    ctx->codec_id = codecpar->codec_id;
    ctx->nal_size = 0;
    ctx->format = AMediaFormat_new();
    ctx->pix_format = AV_PIX_FMT_NONE;
//    "video/x-vnd.on2.vp8" - VP8 video (i.e. video in .webm)
//    "video/x-vnd.on2.vp9" - VP9 video (i.e. video in .webm)
//    "video/avc" - H.264/AVC video
//    "video/hevc" - H.265/HEVC video
//    "video/mp4v-es" - MPEG4 video
//    "video/3gpp" - H.263 video
    switch (ctx->codec_id) {
        case AV_CODEC_ID_H264:
            ctx->codec = AMediaCodec_createDecoderByType("video/avc");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/avc");
            if (codecpar->extradata[0] == 1) {
                size_t sps_size, pps_size;
                uint8_t *sps_buf;
                uint8_t *pps_buf;
                sps_buf = (uint8_t *) malloc((size_t) codecpar->extradata_size + 20);
                pps_buf = (uint8_t *) malloc((size_t) codecpar->extradata_size + 20);
                if (0 != convert_sps_pps2(codecpar->extradata, (size_t) codecpar->extradata_size,
                                          sps_buf, &sps_size, pps_buf, &pps_size, &ctx->nal_size)) {
                    LOGE("%s:convert_sps_pps: failed\n", __func__);
                }
                AMediaFormat_setBuffer(ctx->format, "csd-0", sps_buf, sps_size);
                AMediaFormat_setBuffer(ctx->format, "csd-1", pps_buf, pps_size);
                free(sps_buf);
                free(pps_buf);
            }
            break;
        case AV_CODEC_ID_HEVC:
            ctx->codec = AMediaCodec_createDecoderByType("video/hevc");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/hevc");
            if (codecpar->extradata_size > 3 &&
                (codecpar->extradata[0] == 1 || codecpar->extradata[1] == 1)) {
                size_t sps_pps_size = 0;
                size_t convert_size = (size_t) (codecpar->extradata_size + 20);
                uint8_t *convert_buf = (uint8_t *) malloc((size_t) convert_size);
                if (0 != convert_hevc_nal_units(codecpar->extradata, (size_t) codecpar->extradata_size,
                                                convert_buf, convert_size, &sps_pps_size,
                                                &ctx->nal_size)) {
                    LOGE("%s:convert_sps_pps: failed\n", __func__);
                }
                AMediaFormat_setBuffer(ctx->format, "csd-0", convert_buf, sps_pps_size);
                free(convert_buf);
            }
            break;
        case AV_CODEC_ID_MPEG4:
            ctx->codec = AMediaCodec_createDecoderByType("video/mp4v-es");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/mp4v-es");
            AMediaFormat_setBuffer(ctx->format, "csd-0", codecpar->extradata,
                                   (size_t) codecpar->extradata_size);
            break;
        case AV_CODEC_ID_VP8:
            ctx->codec = AMediaCodec_createDecoderByType("video/x-vnd.on2.vp8");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/x-vnd.on2.vp8");
            break;
        case AV_CODEC_ID_VP9:
            ctx->codec = AMediaCodec_createDecoderByType("video/x-vnd.on2.vp9");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/x-vnd.on2.vp9");
            break;
        case AV_CODEC_ID_H263:
            ctx->codec = AMediaCodec_createDecoderByType("video/3gpp");
            AMediaFormat_setString(ctx->format, AMEDIAFORMAT_KEY_MIME, "video/3gpp");
            AMediaFormat_setBuffer(ctx->format, "csd-0", codecpar->extradata,
                                   (size_t) codecpar->extradata_size);
            break;
        default:
            break;
    }
//    AMediaFormat_setInt32(ctx->format, "rotation-degrees", 90);
    AMediaFormat_setInt32(ctx->format, AMEDIAFORMAT_KEY_WIDTH, ctx->width);
    AMediaFormat_setInt32(ctx->format, AMEDIAFORMAT_KEY_HEIGHT, ctx->height);
//    media_status_t ret = AMediaCodec_configure(ctx->codec, ctx->format, NULL, NULL, 0);

    return ctx;
}

void xl_mediacodec_start(xl_play_data *pd){
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    while(pd->video_render_ctx->texture_window == NULL){
        usleep(10000);
    }
    media_status_t ret = AMediaCodec_configure(ctx->codec, ctx->format, pd->video_render_ctx->texture_window, NULL, 0);
    if (ret != AMEDIA_OK) {
        LOGE("open mediacodec failed \n");
    }
    ret = AMediaCodec_start(ctx->codec);
    if (ret != AMEDIA_OK) {
        LOGE("open mediacodec failed \n");
    }
}

void xl_mediacodec_stop(xl_play_data * pd){
    AMediaCodec_stop(pd->pMediaCodecCtx->codec);
}

void xl_mediacodec_release_context(xl_play_data * pd){
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    AMediaCodec_delete(ctx->codec);
    AMediaFormat_delete(ctx->format);
    free(ctx);
    pd->pMediaCodecCtx = NULL;
}
#else


static int get_int(uint8_t *buf) {
    return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

static int64_t get_long(uint8_t *buf) {
    return (((int64_t) buf[0]) << 56)
           + (((int64_t) buf[1]) << 48)
           + (((int64_t) buf[2]) << 40)
           + (((int64_t) buf[3]) << 32)
           + (((int64_t) buf[4]) << 24)
           + (((int64_t) buf[5]) << 16)
           + (((int64_t) buf[6]) << 8)
           + ((int64_t) buf[7]);
}

xl_mediacodec_context *xl_create_mediacodec_context(
        xl_play_data *pd) {
    xl_mediacodec_context *ctx = (xl_mediacodec_context *) malloc(sizeof(xl_mediacodec_context));
    AVCodecParameters *codecpar = pd->pFormatCtx->streams[pd->videoIndex]->codecpar;
    ctx->width = codecpar->width;
    ctx->height = codecpar->height;
    ctx->codec_id = codecpar->codec_id;
    ctx->nal_size = 0;
    ctx->pix_format = AV_PIX_FMT_NONE;
    return ctx;
}

void xl_mediacodec_start(xl_play_data *pd){
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    JNIEnv *jniEnv = ctx->jniEnv;
    xl_java_class * jc = pd->jc;
    AVCodecParameters *codecpar = pd->pFormatCtx->streams[pd->videoIndex]->codecpar;
    jobject codecName = NULL, csd_0 = NULL, csd_1 = NULL;
    while(pd->video_render_ctx->texture_window == NULL){
        usleep(10000);
    }
    switch (ctx->codec_id) {
        case AV_CODEC_ID_H264:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/avc");
            if (codecpar->extradata[0] == 1) {
                size_t sps_size, pps_size;
                uint8_t *sps_buf;
                uint8_t *pps_buf;
                sps_buf = (uint8_t *) malloc((size_t) codecpar->extradata_size + 20);
                pps_buf = (uint8_t *) malloc((size_t) codecpar->extradata_size + 20);
                if (0 != convert_sps_pps2(codecpar->extradata, (size_t) codecpar->extradata_size,
                                          sps_buf, &sps_size, pps_buf, &pps_size, &ctx->nal_size)) {
                    LOGE("%s:convert_sps_pps: failed\n", __func__);
                }
                csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, sps_buf, sps_size);
                csd_1 = (*jniEnv)->NewDirectByteBuffer(jniEnv, pps_buf, pps_size);
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init,
                                                codecName, ctx->width, ctx->height, csd_0, csd_1);
                free(sps_buf);
                free(pps_buf);
                (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);
                (*jniEnv)->DeleteLocalRef(jniEnv, csd_1);
            } else {
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init,
                                                codecName, ctx->width, ctx->height, NULL, NULL);
            }
            break;
        case AV_CODEC_ID_HEVC:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/hevc");
            if (codecpar->extradata_size > 3 &&
                (codecpar->extradata[0] == 1 || codecpar->extradata[1] == 1)) {
                size_t sps_pps_size = 0;
                size_t convert_size = (size_t) (codecpar->extradata_size + 20);
                uint8_t *convert_buf = (uint8_t *) malloc((size_t) convert_size);
                if (0 !=
                    convert_hevc_nal_units(codecpar->extradata, (size_t) codecpar->extradata_size,
                                           convert_buf, convert_size, &sps_pps_size,
                                           &ctx->nal_size)) {
                    LOGE("%s:convert_sps_pps: failed\n", __func__);
                }
                csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, convert_buf, sps_pps_size);
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init,
                                                codecName, ctx->width, ctx->height, csd_0, NULL);
                free(convert_buf);
                (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);
            }else{
                (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init, codecName,
                                                ctx->width, ctx->height, NULL, NULL);
            }
            break;
        case AV_CODEC_ID_MPEG4:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/mp4v-es");
            csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, codecpar->extradata,
                                                   (jlong) (codecpar->extradata_size));
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init, codecName,
                                            ctx->width, ctx->height, csd_0, NULL);
            (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);
            break;
        case AV_CODEC_ID_VP8:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/x-vnd.on2.vp8");
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init, codecName,
                                            ctx->width, ctx->height, NULL, NULL);
            break;
        case AV_CODEC_ID_VP9:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/x-vnd.on2.vp9");
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init, codecName,
                                            ctx->width, ctx->height, NULL, NULL);
            break;
        case AV_CODEC_ID_H263:
            codecName = (*jniEnv)->NewStringUTF(jniEnv, "video/3gpp");
            csd_0 = (*jniEnv)->NewDirectByteBuffer(jniEnv, codecpar->extradata,
                                                   (jlong) (codecpar->extradata_size));
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_init, codecName,
                                            ctx->width, ctx->height, csd_0, NULL);
            (*jniEnv)->DeleteLocalRef(jniEnv, csd_0);
            break;
        default:
            break;
    }
    if (codecName != NULL) {
        (*jniEnv)->DeleteLocalRef(jniEnv, codecName);
    }
}

void xl_mediacodec_release_buffer(xl_play_data *pd, AVFrame *frame) {
    JNIEnv *jniEnv = pd->video_render_ctx->jniEnv;
    xl_java_class * jc = pd->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_releaseOutPutBuffer,
                                    frame->HW_BUFFER_ID);
}

int xl_mediacodec_receive_frame(xl_play_data *pd, AVFrame *frame) {
    JNIEnv *jniEnv = pd->pMediaCodecCtx->jniEnv;
    xl_java_class * jc = pd->jc;
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    int output_ret = 1;
    jobject deqret = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
                                                       jc->codec_dequeueOutputBufferIndex,
                                                       (jlong) 0);
    uint8_t *retbuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, deqret);
    int outbufidx = get_int(retbuf);
//    int offset = get_int(retbuf + 4);
    int64_t pts = get_long(retbuf + 8);
    (*jniEnv)->DeleteLocalRef(jniEnv, deqret);

    if (outbufidx >= 0) {
//        jobject obuf = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
//                                                         jc->codec_getOutputBuffer, outbufidx);
//        uint8_t *outputBuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, obuf);
//        int size = av_image_get_buffer_size(ctx->pix_format, ctx->width, ctx->height, 1);
        frame->pts = pts;
        frame->format = XL_PIX_FMT_EGL_EXT;
        frame->width = pd->width;
        frame->linesize[0] = pd->width;
//        frame->linesize[0] = ctx->width;
        frame->height = pd->height;
        frame->HW_BUFFER_ID = outbufidx;
//        frame->data[0] = outputBuf + offset;
//        (*jniEnv)->DeleteLocalRef(jniEnv, obuf);

        // use ctx->height  to reslove  ctx->height != pd->height
//        switch (ctx->pix_format) {
//            case AV_PIX_FMT_YUV420P:
//                frame->data[1] = frame->data[0] + frame->linesize[0] * ctx->height;
//                frame->linesize[1] = frame->linesize[0] / 2;
//                frame->data[2] = frame->data[1] + frame->linesize[0] * ctx->height / 4;
//                frame->linesize[2] = frame->linesize[0] / 2;
//                break;
//            case AV_PIX_FMT_NV12:
//                frame->data[1] = frame->data[0] + frame->linesize[0] * ctx->height;
//                frame->linesize[1] = frame->linesize[0];
//                break;
//            default:
//                break;
//        }
        output_ret = 0;
    } else {
        switch (outbufidx) {
            // AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED
            case -2: {
                jobject newFormat = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
                                                                      jc->codec_formatChange);
                uint8_t *fmtbuf = (*jniEnv)->GetDirectBufferAddress(jniEnv, newFormat);
                ctx->width = get_int(fmtbuf);
                ctx->height = get_int(fmtbuf + 4);
                int pix_format = get_int(fmtbuf + 8);
                (*jniEnv)->DeleteLocalRef(jniEnv, newFormat);

                //todo 仅支持了两种格式
                switch (pix_format) {
                    case 19:
                        ctx->pix_format = AV_PIX_FMT_YUV420P;
                        break;
                    case 21:
                        ctx->pix_format = AV_PIX_FMT_NV12;
                        break;
                    default:
                        break;
                }
                output_ret = -2;
                break;
            }
            // AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED
            case -3:
                break;
            // AMEDIACODEC_INFO_TRY_AGAIN_LATER
            case -1:
                break;
            default:
                break;
        }

    }
    return output_ret;
}

int xl_mediacodec_send_packet(xl_play_data *pd, AVPacket *packet) {
    JNIEnv *jniEnv = pd->pMediaCodecCtx->jniEnv;
    xl_java_class * jc = pd->jc;
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    if (packet == NULL) { return -2; }
    int keyframe_flag = 0;
//    av_packet_split_side_data(packet);
    int64_t time_stamp = packet->pts;
    if (!time_stamp && packet->dts)
        time_stamp = packet->dts;
    if (time_stamp > 0) {
        time_stamp = av_rescale_q(time_stamp, pd->pFormatCtx->streams[pd->videoIndex]->time_base,
                                  AV_TIME_BASE_Q);
    } else {
        time_stamp = 0;
    }
    if (ctx->codec_id == AV_CODEC_ID_H264 ||
        ctx->codec_id == AV_CODEC_ID_HEVC) {
        H264ConvertState convert_state = {0, 0};
        convert_h264_to_annexb(packet->data, packet->size, ctx->nal_size, &convert_state);
    }
    if ((packet->flags | AV_PKT_FLAG_KEY) > 0) {
        keyframe_flag |= 0x1;
    }
    // todo : 这个地方会卡
//    ssize_t id = AMediaCodec_dequeueInputBuffer(ctx->codec, 1000000);
    int id = (*jniEnv)->CallStaticIntMethod(jniEnv, jc->HwDecodeBridge,
                                            jc->codec_dequeueInputBuffer, (jlong) 1000000);
    if (id >= 0) {
        jobject inputBuffer = (*jniEnv)->CallStaticObjectMethod(jniEnv, jc->HwDecodeBridge,
                                                                jc->codec_getInputBuffer, id);
        uint8_t *buf = (*jniEnv)->GetDirectBufferAddress(jniEnv, inputBuffer);
        jlong size = (*jniEnv)->GetDirectBufferCapacity(jniEnv, inputBuffer);
//        uint8_t *buf = AMediaCodec_getInputBuffer(ctx->codec, id, &size);
        if (buf != NULL && size >= packet->size) {
            memcpy(buf, packet->data, (size_t) packet->size);
//            media_status = AMediaCodec_queueInputBuffer(ctx->codec, id, 0, packet->size, time_stamp,
//                                                        keyframe_flag);
            (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge,
                                            jc->codec_queueInputBuffer,
                                            (jint) id, (jint) packet->size,
                                            (jlong) time_stamp, (jint) keyframe_flag);
        }
        (*jniEnv)->DeleteLocalRef(jniEnv, inputBuffer);
        // AMEDIACODEC_INFO_TRY_AGAIN_LATER
    } else if (id == -1) {
        return -1;
    } else {
        LOGE("input buffer id < 0  value == %zd", id);
    }
    return 0;
}

void xl_mediacodec_flush(xl_play_data *pd) {
    JNIEnv *jniEnv = pd->pMediaCodecCtx->jniEnv;
    xl_java_class * jc = pd->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_flush);
}

void xl_mediacodec_release_context(xl_play_data *pd) {
    JNIEnv *jniEnv = pd->jniEnv;
    xl_java_class * jc = pd->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_release);
    xl_mediacodec_context *ctx = pd->pMediaCodecCtx;
    free(ctx);
    pd->pMediaCodecCtx = NULL;
}

void xl_mediacodec_stop(xl_play_data *pd) {
    JNIEnv *jniEnv = pd->pMediaCodecCtx->jniEnv;
    xl_java_class * jc = pd->jc;
    (*jniEnv)->CallStaticVoidMethod(jniEnv, jc->HwDecodeBridge, jc->codec_stop);
}

#endif