//
// Created by gutou on 2017/4/11.
//

#ifndef WP_ANDROID_SO3UTIL_H
#define WP_ANDROID_SO3UTIL_H

#include "Vector3d.h"
#include "Matrix3x3d.h"


class SO3Util {
public:
    static void so3FromTwoVecN(Vector3d *a, Vector3d *b, Matrix3x3d *result);
    static void rotationPiAboutAxis(Vector3d *v, Matrix3x3d *result);
    static void so3FromMu(Vector3d *w, Matrix3x3d *result);
    static void muFromSO3(Matrix3x3d *so3, Vector3d *result);
    static void rodriguesSo3Exp(Vector3d *w, double kA, double kB, Matrix3x3d *result);
    __attribute__((unused))
    static void generatorField(int i, Matrix3x3d *pos, Matrix3x3d *result);
};


#endif //WP_ANDROID_SO3UTIL_H
