//
// Created by 晓龙同学 on 2017/4/12.
//

#ifndef XL_XL_MESH_FACTORY_H
#define XL_XL_MESH_FACTORY_H

typedef struct {
    float* pp;
    float* tt;
    unsigned int * index;
    int ppLen, ttLen, indexLen;
} xl_mesh;


xl_mesh *get_ball_mesh();
xl_mesh *get_rect_mesh();
xl_mesh *get_planet_mesh();
xl_mesh *get_distortion_mesh(int eye);

void free_mesh(xl_mesh *p);

#endif //XL_XL_MESH_FACTORY_H
