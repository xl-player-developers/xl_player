//
// Created by gutou on 2017/4/18.
//

#include "xl_packet_queue.h"
#include "xl_packet_pool.h"
static void double_size(xl_packet_queue *queue){
    AVPacket ** temp_packets = (AVPacket **)malloc(sizeof(AVPacket *) * queue->size * 2);
    if(queue->writeIndex == 0){
        memcpy(temp_packets, queue->packets, sizeof(AVPacket *) * queue->size);
        queue->writeIndex = queue->size;
    }else{
        memcpy(temp_packets, queue->packets, sizeof(AVPacket *) * queue->writeIndex);
        memcpy(temp_packets + (queue->writeIndex + queue->size),
               queue->packets + queue->writeIndex,
               sizeof(AVPacket *) * (queue->size - queue->readIndex));
        queue->readIndex += queue->size;
    }
    free(queue->packets);
    queue->packets = temp_packets;
    queue->size *= 2;
}

xl_packet_queue * xl_queue_create(unsigned int size){
    xl_packet_queue * queue = (xl_packet_queue *)malloc(sizeof(xl_packet_queue));
    queue->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    queue->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(queue->mutex, NULL);
    pthread_cond_init(queue->cond, NULL);
    queue->readIndex = 0;
    queue->writeIndex = 0;
    queue->count = 0;
    queue->size = size;
    queue->duration = 0;
    queue->max_duration = 0;
    queue->flush_packet.duration = 0;
    queue->packets = (AVPacket **)malloc(sizeof(AVPacket *) * size);
    queue->full_cb = NULL;
    queue->empty_cb = NULL;
    return queue;
}

void xl_queue_set_duration(xl_packet_queue * queue, uint64_t max_duration){
    queue->max_duration = max_duration;
}

void xl_packet_queue_free(xl_packet_queue *queue){
    pthread_mutex_destroy(queue->mutex);
    pthread_cond_destroy(queue->cond);
    free(queue->packets);
    free(queue);
}

int xl_packet_queue_put(xl_packet_queue *queue, AVPacket *packet){
    pthread_mutex_lock(queue->mutex);
    if(queue->max_duration > 0 && queue->duration + packet->duration > queue->max_duration){
        if(queue->full_cb != NULL){
            queue->full_cb(queue->cb_data);
        }
        pthread_cond_wait(queue->cond, queue->mutex);
    }
    if(queue->count == queue->size){
        double_size(queue);
    }
    queue->duration += packet->duration;
    queue->packets[queue->writeIndex] = packet;
    queue->writeIndex = (queue->writeIndex + 1) % queue->size;
    queue->count++;
    pthread_mutex_unlock(queue->mutex);
    return 0;
}

AVPacket *  xl_packet_queue_get(xl_packet_queue *queue){
    pthread_mutex_lock(queue->mutex);
    if(queue->count == 0){
        pthread_mutex_unlock(queue->mutex);
        if(queue->empty_cb != NULL){
            queue->empty_cb(queue->cb_data);
        }
        return NULL;
    }
    AVPacket * packet = queue->packets[queue->readIndex];
    queue->readIndex = (queue->readIndex + 1) % queue->size;
    queue->count--;
    queue->duration -= packet->duration;
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
    return packet;
}

void xl_packet_queue_flush(xl_packet_queue *queue, xl_pakcet_pool *pool){
    pthread_mutex_lock(queue->mutex);
    while(queue->count > 0){
        AVPacket * packet = queue->packets[queue->readIndex];
        if(packet != &queue->flush_packet){
            xl_packet_pool_unref_packet(pool, packet);
        }
        queue->readIndex = (queue->readIndex + 1) % queue->size;
        queue->count--;
    }
    queue->readIndex = 0;
    queue->duration = 0;
    queue->packets[0] = &queue->flush_packet;
    queue->writeIndex = 1;
    queue->count = 1;
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
}