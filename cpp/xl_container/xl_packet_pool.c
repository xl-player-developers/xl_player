//
// Created by gutou on 2017/4/19.
//
#include "xl_packet_pool.h"

static void double_size(xl_pakcet_pool * pool){
    // step1  malloc new memery space to store pointers |________________|
    AVPacket ** temp_packets = (AVPacket **)av_malloc(sizeof(AVPacket *) * pool->size * 2);
    // step2  copy old pointers to new space            |XXXXXXXX________|
    memcpy(temp_packets, pool->packets, sizeof(AVPacket *) * pool->size);
    // step3 fill rest space with av_packet_alloc       |XXXXXXXXOOOOOOOO|
    for(int i = pool->size; i < pool->size * 2; i++){
        temp_packets[i] = av_packet_alloc();
    }
    // step4 free old pointers space
    free(pool->packets);
    pool->packets = temp_packets;
    // step5 当前指针位置移动到后半部分
    pool->index = pool->size;
    pool->size *= 2;
    LOGI("packet pool double size. new size ==> %d", pool->size);
}

xl_pakcet_pool * xl_packet_pool_create(int size){
    xl_pakcet_pool * pool = (xl_pakcet_pool *)malloc(sizeof(xl_pakcet_pool));
    pool->size = size;
    pool->packets = (AVPacket **)av_malloc(sizeof(AVPacket *) * size);
    for(int i = 0; i < pool->size; i++){
        pool->packets[i] = av_packet_alloc();
    }
    return pool;
}

void xl_packet_pool_reset(xl_pakcet_pool * pool){
    pool->count = 0;
    pool->index = 0;
}

void xl_packet_pool_release(xl_pakcet_pool * pool){
    for(int i = 0; i < pool->size; i++){
        AVPacket * p = pool->packets[i];
        av_packet_free(&p);
    }
    free(pool->packets);
    free(pool);
}
AVPacket * xl_packet_pool_get_packet(xl_pakcet_pool * pool){
    if(pool->count > pool->size / 2){
        double_size(pool);
    }
    AVPacket * p = pool->packets[pool->index];
    pool->index = (pool->index + 1) % pool->size;
    pool->count ++;
    return p;
}
void xl_packet_pool_unref_packet(xl_pakcet_pool * pool, AVPacket * packet){
    av_packet_unref(packet);
    pool->count--;
}