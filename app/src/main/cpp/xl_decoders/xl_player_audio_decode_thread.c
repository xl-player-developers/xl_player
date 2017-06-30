//
// Created by gutou on 2017/5/10.
//

#include <unistd.h>
#include <sys/prctl.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include "xl_player_audio_decode_thread.h"
#include "../xl_container/xl_packet_queue.h"
#include "../xl_container/xl_packet_pool.h"
#include "../xl_container/xl_frame_pool.h"
#include "../xl_container/xl_frame_queue.h"

void * audio_decode_thread(void * data){
    prctl(PR_SET_NAME, __func__);
    xl_play_data * pd = (xl_play_data *)data;
    xl_audio_filter_context * ctx = pd->audio_filter_ctx;
    int ret, filter_ret = 0;
    AVFrame * decode_frame = av_frame_alloc();
    AVFrame * frame = xl_frame_pool_get_frame(pd->audio_frame_pool);
    while (pd->error_code == 0) {
        if(pd->status == PAUSED){
            usleep(NULL_LOOP_SLEEP_US);
        }
        ret = avcodec_receive_frame(pd->audio_codec_ctx, decode_frame);
        if (ret == 0) {
            pthread_mutex_lock(ctx->filter_lock);
            int add_ret = av_buffersrc_add_frame(ctx->buffersrc_ctx, decode_frame);
            if(add_ret >= 0){
                while(1){
                    filter_ret = av_buffersink_get_frame(ctx->buffersink_ctx, frame);
                    if(filter_ret < 0 || filter_ret == AVERROR(EAGAIN) || filter_ret == AVERROR_EOF){
                        break;
                    }
                    xl_frame_queue_put(pd->audio_frame_queue, frame);
                    frame = xl_frame_pool_get_frame(pd->audio_frame_pool);
                }
                // 触发音频播放
                if (pd->av_track_flags & XL_HAS_AUDIO_FLAG){
                    pd->audio_player_ctx->play(pd);
                }

            }else{
                char err[128];
                av_strerror(add_ret, err, 127);
                err[127] = 0;
                LOGE("add to audio filter error ==>\n %s", err);
            }
            pthread_mutex_unlock(ctx->filter_lock);

        } else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            AVPacket *packet = xl_packet_queue_get(pd->audio_packet_queue);
            // buffer empty ==> wait  10ms
            // eof          ==> break
            if(packet == NULL){
                if(pd->eof){
                    break;
                }else{
                    usleep(BUFFER_EMPTY_SLEEP_US);
                    continue;
                }
            }
            // seek
            if (packet == &pd->audio_packet_queue->flush_packet) {
                xl_frame_queue_flush(pd->audio_frame_queue, pd->audio_frame_pool);
                avcodec_flush_buffers(pd->audio_codec_ctx);
                continue;
            }
            ret = avcodec_send_packet(pd->audio_codec_ctx, packet);
            xl_packet_pool_unref_packet(pd->packet_pool, packet);
            if (ret < 0) {
                pd->error_code = 3001;
                break;
            }
        } else if (ret == AVERROR(EINVAL)) {
            pd->error_code = 3002;
            break;
        } else {
            pd->error_code = 3003;
            break;
        }
    }
    av_frame_free(&decode_frame);
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}