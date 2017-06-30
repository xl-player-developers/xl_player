//
// Created by gutou on 2017/5/8.
//

#include <unistd.h>
#include <sys/prctl.h>
#include "xl_player_read_thread.h"
#include "xl_container/xl_packet_queue.h"
#include "xl_container/xl_packet_pool.h"

static inline void flush_packet_queue(xl_play_data *pd) {
    if (pd->av_track_flags & XL_HAS_VIDEO_FLAG) {
        xl_packet_queue_flush(pd->video_packet_queue, pd->packet_pool);
    }
    if (pd->av_track_flags & XL_HAS_AUDIO_FLAG) {
        xl_packet_queue_flush(pd->audio_packet_queue, pd->packet_pool);
    }
}

// 读文件线程
void *read_thread(void *data) {
    prctl(PR_SET_NAME, __func__);
    xl_play_data *pd = (xl_play_data *) data;
    AVPacket *packet = NULL;
    int ret = 0;
    while (pd->error_code == 0) {
        if (pd->seeking == 1) {
            pd->seeking = 2;
            flush_packet_queue(pd);
            int seek_ret = av_seek_frame(pd->pFormatCtx, -1, (int64_t) (pd->seek_to * AV_TIME_BASE),
                                         AVSEEK_FLAG_BACKWARD);
            if (seek_ret < 0) {
                LOGE("seek faild");
            }
        }
        if (pd->audio_packet_queue->total_bytes + pd->video_packet_queue->total_bytes >=
            pd->buffer_size_max) {
            if (pd->status == BUFFER_EMPTY) {
                pd->send_message(pd, xl_message_buffer_full);
            }
            usleep(NULL_LOOP_SLEEP_US);
        }
        // get a new packet from pool
        if (packet == NULL) {
            packet = xl_packet_pool_get_packet(pd->packet_pool);
        }
        // read data to packet
        ret = av_read_frame(pd->pFormatCtx, packet);
        if (ret == 0) {
            if (packet->stream_index == pd->videoIndex) {
                xl_packet_queue_put(pd->video_packet_queue, packet);
                pd->statistics->bytes += packet->size;
                packet = NULL;
            } else if (packet->stream_index == pd->audioIndex) {
                xl_packet_queue_put(pd->audio_packet_queue, packet);
                pd->statistics->bytes += packet->size;
                packet = NULL;
            } else {
                av_packet_unref(packet);
            }
        } else if (ret == AVERROR_INVALIDDATA) {
            xl_packet_pool_unref_packet(pd->packet_pool, packet);
        } else if (ret == AVERROR_EOF) {
            pd->eof = true;
            if (pd->status == BUFFER_EMPTY) {
                pd->send_message(pd, xl_message_buffer_full);
            }
            break;
        } else {
            // error
            pd->error_code = ret;
            LOGE("read file error. error code ==> %d", ret);
            break;
        }
    }
    LOGI("thread ==> %s exit", __func__);
    return NULL;
}