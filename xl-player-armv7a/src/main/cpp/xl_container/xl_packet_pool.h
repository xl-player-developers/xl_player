//
// Created by gutou on 2017/4/19.
//

#ifndef XL_XL_PACKETPOOL_H
#define XL_XL_PACKETPOOL_H
#include "../xl_types/xl_player_types.h"

xl_pakcet_pool * xl_packet_pool_create(int size);
void xl_packet_pool_reset(xl_pakcet_pool * pool);
void xl_packet_pool_release(xl_pakcet_pool * pool);
AVPacket * xl_packet_pool_get_packet(xl_pakcet_pool * pool);
void xl_packet_pool_unref_packet(xl_pakcet_pool * pool, AVPacket * packet);
#endif //XL_XL_PACKETPOOL_H
