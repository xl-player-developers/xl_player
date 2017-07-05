//
// Created by gutou on 2017/4/18.
//

#ifndef XL_XL_PACKET_QUEUE_H
#define XL_XL_PACKET_QUEUE_H

#include <pthread.h>
#include "../xl_types/xl_player_types.h"


xl_packet_queue * xl_queue_create(unsigned int size);
void xl_queue_set_duration(xl_packet_queue * queue, uint64_t max_duration);
void xl_packet_queue_free(xl_packet_queue *queue);
int xl_packet_queue_put(xl_packet_queue *queue, AVPacket *packet);
AVPacket *  xl_packet_queue_get(xl_packet_queue *queue);
void xl_packet_queue_flush(xl_packet_queue *queue, xl_pakcet_pool *pool);
#endif //XL_XL_QUEUE_H
