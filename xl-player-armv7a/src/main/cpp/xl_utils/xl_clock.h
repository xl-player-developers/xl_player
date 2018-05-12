//
// Created by gutou on 2017/4/20.
//

#ifndef XL_XL_CLOCK_H
#define XL_XL_CLOCK_H


#include <stdint.h>
#include "../xl_types/xl_player_types.h"


uint64_t xl_clock_get_current_time();
xl_clock * xl_clock_creat();
int64_t xl_clock_get(xl_clock * clock);
void xl_clock_set(xl_clock * clock, int64_t pts);
void xl_clock_free(xl_clock * clock);
void xl_clock_reset(xl_clock * clock);
#endif //XL_XL_CLOCK_H
