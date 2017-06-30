//
// Created by gutou on 2017/5/8.
//

#include <unistd.h>
#include <sys/prctl.h>
#include "xl_player_video_hw_decode_thread.h"
#include "xl_mediacodec.h"
#include "../xl_container/xl_packet_queue.h"
#include "../xl_container/xl_packet_pool.h"
#include "../xl_container/xl_frame_pool.h"
#include "../xl_container/xl_frame_queue.h"

static inline int drop_video_packet(xl_play_data * pd){
    AVPacket * packet = xl_packet_queue_get(pd->video_packet_queue);
    if(packet != NULL){
        if (packet != &pd->video_packet_queue->flush_packet){
            int64_t time_stamp = av_rescale_q(packet->pts,
                                              pd->pFormatCtx->streams[pd->videoIndex]->time_base,
                                              AV_TIME_BASE_Q);
            xl_packet_pool_unref_packet(pd->packet_pool, packet);
            int64_t diff = time_stamp - pd->audio_clock->pts;
            if(diff > 0){
                usleep((useconds_t) diff);
            }
        } else {
            xl_frame_queue_flush(pd->video_frame_queue, pd->video_frame_pool);
            xl_mediacodec_flush(pd);
        }
    }else{
        if(pd->eof){
            return -1;
        }
        usleep(NULL_LOOP_SLEEP_US);
    }
    return 0;
}

void * video_decode_hw_thread(void * data){
    prctl(PR_SET_NAME, __func__);
    xl_play_data * pd = (xl_play_data *)data;
    (*pd->vm)->AttachCurrentThread(pd->vm, &pd->pMediaCodecCtx->jniEnv, NULL);
    xl_mediacodec_start(pd);
    int ret;
    AVPacket * packet = NULL;
    AVFrame * frame = xl_frame_pool_get_frame(pd->video_frame_pool);
    while (pd->error_code == 0) {
        if(pd->just_audio){
            // 如果只播放音频  按照音视频同步的速度丢包
            if( -1 == drop_video_packet(pd)){
                break;
            }
        }else{
            ret = xl_mediacodec_receive_frame(pd, frame);
            if (ret == 0) {
                frame->FRAME_ROTATION = pd->frame_rotation;
                xl_frame_queue_put(pd->video_frame_queue, frame);
                frame = xl_frame_pool_get_frame(pd->video_frame_pool);
            }else if(ret == 1) {
                if(packet == NULL){
                    packet = xl_packet_queue_get(pd->video_packet_queue);
                }
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
                if(packet == &pd->video_packet_queue->flush_packet){
                    xl_frame_queue_flush(pd->video_frame_queue, pd->video_frame_pool);
                    xl_mediacodec_flush(pd);
                    packet = NULL;
                    continue;
                }
                if(0 == xl_mediacodec_send_packet(pd, packet)){
                    xl_packet_pool_unref_packet(pd->packet_pool, packet);
                    packet = NULL;
                }else{
                    // some device AMediacodec input buffer ids count < frame_queue->size
                    // when pause   frame_queue not full
                    // thread will not block in  "xl_frame_queue_put" function
                    if(pd->status == PAUSED){
                        usleep(NULL_LOOP_SLEEP_US);
                    }
                }

            }else if(ret == -2) {
                //frame = xl_frame_pool_get_frame(pd->video_frame_pool);
            }else {
                pd->error_code = 501;
                break;
            }
        }
    }
    xl_mediacodec_stop(pd);
    (*pd->vm)->DetachCurrentThread(pd->vm);
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}