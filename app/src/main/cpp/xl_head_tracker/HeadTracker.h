//
// Created by gutou on 2017/6/1.
//

#ifndef XL_HEADTRACKER_H
#define XL_HEADTRACKER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void xl_ekf_reset();
void xl_ekf_get_predicted_matrix(double secondsAfterLastGyroEvent, float *matrix);
void xl_ekf_process_acc(float x, float y, float z, int64_t ts);
void xl_ekf_process_gyro(float x, float y, float z, int64_t ts);

#ifdef __cplusplus
}
#endif
#endif //XL_HEADTRACKER_H
