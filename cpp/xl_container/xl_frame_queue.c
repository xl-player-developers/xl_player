//
// Created by gutou on 2017/5/8.
//

#include <pthread.h>
#include "xl_frame_queue.h"
#include "xl_frame_pool.h"

xl_frame_queue * xl_frame_queue_create(unsigned int size){
    xl_frame_queue * queue = (xl_frame_queue *)malloc(sizeof(xl_frame_queue));
    queue->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    queue->cond = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(queue->mutex, NULL);
    pthread_cond_init(queue->cond, NULL);
    queue->readIndex = 0;
    queue->writeIndex = 0;
    queue->count = 0;
    queue->size = size;
    queue->frames = (AVFrame **)malloc(sizeof(AVFrame *) * size);
    return queue;
}
void xl_frame_queue_free(xl_frame_queue *queue){
    pthread_mutex_destroy(queue->mutex);
    pthread_cond_destroy(queue->cond);
    free(queue->frames);
    free(queue);
}

int xl_frame_queue_put(xl_frame_queue *queue, AVFrame *frame){
    pthread_mutex_lock(queue->mutex);
    while(queue->count == queue->size){
        pthread_cond_wait(queue->cond, queue->mutex);
    }
    queue->frames[queue->writeIndex] = frame;
    queue->writeIndex = (queue->writeIndex + 1) % queue->size;
    queue->count++;
    pthread_mutex_unlock(queue->mutex);
    return 0;
}

AVFrame *  xl_frame_queue_get(xl_frame_queue *queue){
    pthread_mutex_lock(queue->mutex);
    if(queue->count == 0){
        pthread_mutex_unlock(queue->mutex);
        return NULL;
    }
    AVFrame * frame = queue->frames[queue->readIndex];
    queue->readIndex = (queue->readIndex + 1) % queue->size;
    queue->count--;
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
    return frame;
}

void xl_frame_queue_flush(xl_frame_queue *queue, xl_frame_pool *pool){
    pthread_mutex_lock(queue->mutex);
    while(queue->count > 0){
        AVFrame * frame = queue->frames[queue->readIndex];
        if(frame != &queue->flush_frame){
            xl_frame_pool_unref_frame(pool, frame);
        }
        queue->readIndex = (queue->readIndex + 1) % queue->size;
        queue->count--;
    }
    queue->readIndex = 0;
    queue->frames[0] = &queue->flush_frame;
    queue->writeIndex = 1;
    queue->count = 1;
    pthread_cond_signal(queue->cond);
    pthread_mutex_unlock(queue->mutex);
}