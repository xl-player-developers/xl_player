//
// Created by tianchi on 17-6-21.
//

#ifndef XL_XL_JNI_REFLECT_H
#define XL_XL_JNI_REFLECT_H
#include "../xl_types/xl_player_types.h"

void xl_jni_reflect_java_class(xl_java_class ** p_jc, JNIEnv *jniEnv);
void xl_jni_free(xl_java_class **p_jc, JNIEnv *jniEnv);
#endif //XL_XL_JNI_REFLECT_H
