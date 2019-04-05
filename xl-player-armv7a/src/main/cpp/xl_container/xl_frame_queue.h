//
// Created by gutou on 2017/5/8.
//

#ifndef XL_XL_FRAME_QUEUE_H
#define XL_XL_FRAME_QUEUE_H
#include "../xl_types/xl_player_types.h"
xl_frame_queue * xl_frame_queue_create(unsigned int size);
void xl_frame_queue_free(xl_frame_queue *queue);
int xl_frame_queue_put(xl_frame_queue *queue, AVFrame *frame);
AVFrame *  xl_frame_queue_get(xl_frame_queue *queue);
void xl_frame_queue_flush(xl_frame_queue *queue, xl_frame_pool *pool);
#endif //XL_XL_FRAME_QUEUE_H
