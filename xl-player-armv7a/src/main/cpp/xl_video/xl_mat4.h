//
// Created by gutou on 2017/4/10.
//

#include "../xl_types/xl_video_render_types.h"

#ifndef WP_ANDROID_XL_MATH_H
#define WP_ANDROID_XL_MATH_H

void identity(GLfloat * out);
void perspective (GLfloat fovy, GLfloat aspect, GLfloat near, GLfloat far, GLfloat * out);
void lookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
            GLfloat centerx, GLfloat centery, GLfloat centerz,
            GLfloat upx, GLfloat upy, GLfloat upz,
            GLfloat * out);
__attribute__((unused))
void flipX(GLfloat * a);
void rotateX(GLfloat * out, GLfloat rad);
void rotateY(GLfloat * out, GLfloat rad);
void rotateZ(GLfloat * out, GLfloat rad);
void multiply(GLfloat * out, GLfloat * a, GLfloat * b);

__attribute__((unused))
void setRotateEulerM(GLfloat * out, GLfloat x, GLfloat y, GLfloat z);
__attribute__((unused))
void clone(GLfloat * out, GLfloat * a);
__attribute__((unused))
void transformVec4(GLfloat * out, GLfloat * a, GLfloat * m);
#endif //WP_ANDROID_XL_MATH_H
