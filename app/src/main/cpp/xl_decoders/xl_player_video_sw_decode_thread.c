//
// Created by gutou on 2017/5/8.
//

#include <unistd.h>
#include <sys/prctl.h>
#include "xl_player_video_sw_decode_thread.h"
#include "../xl_types/xl_player_types.h"
#include "../xl_container/xl_packet_pool.h"
#include "../xl_container/xl_packet_queue.h"
#include "../xl_container/xl_frame_pool.h"
#include "../xl_container/xl_frame_queue.h"

void static inline drop_video_packet(xl_play_data * pd){
    AVPacket * packet = xl_packet_queue_get(pd->video_packet_queue);
    if(packet != NULL){
        int64_t time_stamp = av_rescale_q(packet->pts,
                                          pd->pFormatCtx->streams[pd->videoIndex]->time_base,
                                          AV_TIME_BASE_Q);
        xl_packet_pool_unref_packet(pd->packet_pool, packet);
        int64_t diff = time_stamp - pd->audio_clock->pts;
        if(diff > 0){
            usleep((useconds_t) diff);
        }
    }else{
        usleep(NULL_LOOP_SLEEP_US);
    }
}

void * video_decode_sw_thread(void * data){
    prctl(PR_SET_NAME, __func__);
    xl_play_data * pd = (xl_play_data *)data;
    int ret;
    AVFrame * frame = xl_frame_pool_get_frame(pd->video_frame_pool);
    while (pd->error_code == 0) {
        if(pd->just_audio){
            // 如果之播放音频  按照音视频同步的速度丢包
            drop_video_packet(pd);
        }else {
            ret = avcodec_receive_frame(pd->pVideoCodecCtx, frame);
            if (ret == 0) {
                frame->FRAME_ROTATION = pd->frame_rotation;
                xl_frame_queue_put(pd->video_frame_queue, frame);
                usleep(2000);
                frame = xl_frame_pool_get_frame(pd->video_frame_pool);
            } else if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                AVPacket *packet = xl_packet_queue_get(pd->video_packet_queue);
                // buffer empty ==> wait  10ms
                // eof          ==> break
                if (packet == NULL) {
                    if (pd->eof) {
                        break;
                    } else {
                        LOGE("video buffer empty!!!!!!!!!");
                        usleep(BUFFER_EMPTY_SLEEP_US);
                        continue;
                    }
                }
                // seek
                if (packet == &pd->video_packet_queue->flush_packet) {
                    xl_frame_queue_flush(pd->video_frame_queue, pd->video_frame_pool);
                    avcodec_flush_buffers(pd->pVideoCodecCtx);
                    continue;
                }
                ret = avcodec_send_packet(pd->pVideoCodecCtx, packet);
                xl_packet_pool_unref_packet(pd->packet_pool, packet);
                if (ret < 0) {
                    pd->error_code = 4101;
                    break;
                }
            } else if (ret == AVERROR(EINVAL)) {
                pd->error_code = 4102;
                break;
            } else {
                pd->error_code = 4103;
                break;
            }
        }
    }
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}