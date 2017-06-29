//
// Created by gutou on 2017/4/12.
//
//#include "jni.h"
#include <string.h>
#include "OrientationEKF.h"

static OrientationEKF ekf;
extern "C" {
/*
JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_reset(JNIEnv *env, jobject instance) {
    ekf.reset();
}
JNIEXPORT jdoubleArray JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_getPredictedGLMatrix(JNIEnv *env,
                                                                            jobject instance,
                                                                            jdouble secondsAfterLastGyroEvent) {
    double mat[16];
    ekf.getPredictedGLMatrix(secondsAfterLastGyroEvent, mat);
    jdoubleArray ret = env->NewDoubleArray(16);
    env->SetDoubleArrayRegion(ret, 0, 16, mat);
    return ret;
}
JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_processAcc(JNIEnv *env, jobject instance,
                                                                  jfloat x, jfloat y,
                                                                  jfloat z, jlong ts) {
    Vector3d v((double) x, (double) y, (double) z);
    ekf.processAcceleration(v, (double) ts);
}
JNIEXPORT void JNICALL
Java_com_xl_media_library_base_BaseNativeInterface_processGyro(JNIEnv *env, jobject instance,
                                                                   jfloat x, jfloat y,
                                                                   jfloat z, jlong ts) {
    Vector3d v((double) x, (double) y, (double) z);
    ekf.processGyro(v, (double) ts);
}
*/
void xl_ekf_reset(){
    ekf.reset();
}

void xl_ekf_get_predicted_matrix(double secondsAfterLastGyroEvent, float *matrix){
    double mat[16];
    ekf.getPredictedGLMatrix(secondsAfterLastGyroEvent, mat);
    for (int i = 0; i < 16; ++i) {
        matrix[i] = (float)mat[i];
    }
}
void xl_ekf_process_acc(float x, float y, float z, int64_t ts) {
    Vector3d v((double) x, (double) y, (double) z);
    ekf.processAcceleration(v, (double) ts);
}
void xl_ekf_process_gyro(float x, float y, float z, int64_t ts) {
    Vector3d v((double) x, (double) y, (double) z);
    ekf.processGyro(v, (double) ts);
}
}