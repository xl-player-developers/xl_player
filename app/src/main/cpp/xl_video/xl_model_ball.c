//
// Created by gutou on 2017/5/5.
//

#include "xl_model_ball.h"
#include "xl_mesh_factory.h"
#include "xl_mat4.h"
#include "xl_texture.h"
#include "xl_glsl_program.h"

static AAssetManager *pManager;
static GLfloat _rx, _ry, _rz;
static GLfloat _eyez;
static GLfloat _fovy = 1, _w = 1, _h = 1;

static void updateModelMatrix(xl_model *model) {
    identity(model->modelMatrix);
    rotateZ(model->modelMatrix, _rz);
    rotateX(model->modelMatrix, _rx);
    rotateY(model->modelMatrix, _ry);
}

static void updateViewMatrix(xl_model *model) {
    lookAt(0, 0, _eyez, 0, 0, -1, 0, 1, 0, model->view_matrix);
}

static void updateProjectionMatrix(xl_model *model) {
    perspective(_fovy, _w / _h, 0.01, 10, model->projectionMatrix);
}

static void initBallMatrix(xl_model *model) {
    _rx = _ry = _rz = 0;
    _eyez = 0;
    _fovy = 60;
    identity(model->modelMatrix);
    updateViewMatrix(model);
}

static void initPlanetMatrix(xl_model *model) {
    _rx = _ry = _rz = 0;
    _eyez = 0.333333f;
    _fovy = 60;
    identity(model->modelMatrix);
    updateViewMatrix(model);
}

static void resize(xl_model *model, int w, int h) {
    model->viewport_w = w;
    model->viewport_h = h;
    _w = w;
    _h = h;
    updateProjectionMatrix(model);
}

static void updateRotation(xl_model *model, GLfloat rx, GLfloat ry, GLfloat rz,bool enable_tracker) {
    if (_rx != rx || _ry != ry || _rz != rz) {
        _rx += rx;
        _ry += ry;
        _rz += rz;
        if (!enable_tracker){
            updateModelMatrix(model);
        }
    }
}

static void updateEyePosition(xl_model *model, GLfloat eyez) {
    if (eyez != _eyez) {
        _eyez = eyez;
        updateViewMatrix(model);
    }
}


static void updateFov(xl_model *model, GLfloat fov) {
    if (_fovy != fov) {
        _fovy = fov;
        updateProjectionMatrix(model);
    }
}

static void updateHead(xl_model *model, GLfloat *headMatrix) {
    for (int i = 0; i < 16; ++i) {
        model->modelMatrix[i] = headMatrix[i];
    }
    rotateY(model->modelMatrix, _ry);
}

static void draw_ball(xl_model *model) {
    xl_glsl_program *program = model->program;
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, model->viewport_w, model->viewport_h);
    model->bind_texture(model);
    glUniform1f(program->linesize_adjustment_location, model->width_adjustment);
    glUniformMatrix4fv(program->modelMatrixLoc, 1, GL_FALSE, model->modelMatrix);
    glUniformMatrix4fv(program->viewMatrixLoc, 1, GL_FALSE, model->view_matrix);
    glUniformMatrix4fv(program->projectionMatrixLoc, 1, GL_FALSE, model->projectionMatrix);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glDrawElements(GL_TRIANGLES, (GLsizei) model->elementsCount, GL_UNSIGNED_INT, 0);
}

static void draw_expand(xl_model *model) {
    xl_glsl_program *program = model->program;
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, model->viewport_w, model->viewport_h);
    model->bind_texture(model);
    glUniform1f(program->linesize_adjustment_location, model->width_adjustment);
    glUniformMatrix4fv(program->modelMatrixLoc, 1, GL_FALSE, model->modelMatrix);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glDrawElements(GL_TRIANGLES, (GLsizei) model->elementsCount, GL_UNSIGNED_INT, 0);
}

void update_frame_ball(xl_model *model, AVFrame *frame) {
    if (model->pixel_format != frame->format) {
        model->pixel_format = (enum AVPixelFormat) frame->format;
        model->program = xl_glsl_program_get(model->type, model->pixel_format, pManager);
        glUseProgram(model->program->program);
        switch (frame->format) {
            case AV_PIX_FMT_YUV420P:
                model->bind_texture = bind_texture_yuv420p;
                model->updateTexture = update_texture_yuv420p;
                break;
            case AV_PIX_FMT_NV12:
                model->bind_texture = bind_texture_nv12;
                model->updateTexture = update_texture_nv12;
                break;
            case XL_PIX_FMT_EGL_EXT:
                model->bind_texture = bind_texture_oes;
                model->updateTexture = update_texture_oes;
                break;
            default:
                LOGE("not support this pix_format ==> %d", frame->format);
                return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
        glVertexAttribPointer((GLuint) model->program->positon_location, 3, GL_FLOAT, GL_FALSE, 0,
                              0);
        glEnableVertexAttribArray((GLuint) model->program->positon_location);
        glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
        glVertexAttribPointer((GLuint) model->program->texcoord_location, 2, GL_FLOAT, GL_FALSE, 0,
                              0);
        glEnableVertexAttribArray((GLuint) model->program->texcoord_location);
    }
    model->updateTexture(model, frame);
    model->width_adjustment = (float) frame->width / (float) frame->linesize[0];
}

static void update_distance_ball(xl_model *model, GLfloat distance) {
    //取值范围是 [-0.5f, 1f]
    distance = distance > 2 ? 2 : (distance < 0.5f ? 0.5f : distance);
    distance = 1 / distance;
    updateEyePosition(model, distance - 1.0f);
}

static void update_distance_planet(xl_model *model, GLfloat distance) {
    //取值范围是 [-0.5f, 2f]
    distance = distance > 2 ? 2 : (distance < 0.5f ? 0.5f : distance);
    distance = 1 / distance;
    distance = -0.5f + (distance - 0.5f) / 1.5f * 2.5f;
    updateEyePosition(model, distance);
}

static void update_distance_architecture(xl_model *model, GLfloat distance) {
    //取值范围是 [30, 120]
    distance = distance > 2 ? 2 : (distance < 0.5f ? 0.5f : distance);
    distance = 1 / distance;
    updateFov(model, distance * 60);
}

xl_model *model_ball_create(AAssetManager *pAAssetManager) {
    pManager = pAAssetManager;
    xl_model *model = (xl_model *) malloc(sizeof(xl_model));
    model->type = Ball;
    model->program = NULL;
    model->draw = NULL;
    model->updateTexture = NULL;
    model->pixel_format = AV_PIX_FMT_NONE;
    model->resize = resize;
    model->update_frame = update_frame_ball;
    model->draw = draw_ball;
    model->updateModelRotation = updateRotation;
    model->update_distance = update_distance_ball;
//    model->updateEyePosition = updateEyePosition;
//    model->updateEyeCenter = updateEyeCenter;
//    model->updateEyeUp = updateEyeUp;
//    model->updateFov = updateFov;
//    model->updatePerspectiveWH = updatePerspectiveWH;
    model->updateHead = updateHead;
    xl_mesh *ball = get_ball_mesh();
    glGenBuffers(3, model->vbos);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ball->ppLen, ball->pp, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ball->ttLen, ball->tt, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * ball->indexLen, ball->index,
                 GL_STATIC_DRAW);
    model->elementsCount = (size_t) ball->indexLen;
    free_mesh(ball);
    initBallMatrix(model);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    return model;
}

xl_model *model_architecture_create(AAssetManager *pAAssetManager) {
    xl_model *m = model_ball_create(pAAssetManager);
    m->type = Architecture;
    m->update_distance = update_distance_architecture;
    return m;
}

xl_model *model_planet_create(AAssetManager *pAAssetManager) {
    pManager = pAAssetManager;
    xl_model *model = (xl_model *) malloc(sizeof(xl_model));
    model->type = Planet;
    model->program = NULL;
    model->draw = NULL;
    model->updateTexture = NULL;
    model->pixel_format = AV_PIX_FMT_NONE;
    model->resize = resize;
    model->update_frame = update_frame_ball;
    model->draw = draw_ball;
    model->updateModelRotation = updateRotation;
    model->update_distance = update_distance_planet;
    xl_mesh *planet_mesh = get_planet_mesh();
    glGenBuffers(3, model->vbos);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_mesh->ppLen, planet_mesh->pp,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_mesh->ttLen, planet_mesh->tt,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * planet_mesh->indexLen,
                 planet_mesh->index, GL_STATIC_DRAW);
    model->elementsCount = (size_t) planet_mesh->indexLen;
    free_mesh(planet_mesh);
    initPlanetMatrix(model);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    return model;
}

xl_model *model_expand_create(AAssetManager *pAAssetManager) {
    pManager = pAAssetManager;
    xl_model *model = (xl_model *) malloc(sizeof(xl_model));
    model->type = Expand;
    model->program = NULL;
    model->draw = NULL;
    model->updateTexture = NULL;
    model->pixel_format = AV_PIX_FMT_NONE;
    model->resize = resize;
    model->update_frame = update_frame_ball;
    model->draw = draw_expand;
    model->updateModelRotation = updateRotation;
    xl_mesh *ball_mesh = get_ball_mesh();
    glGenBuffers(3, model->vbos);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ball_mesh->ppLen, ball_mesh->pp, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ball_mesh->ttLen, ball_mesh->tt, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * ball_mesh->indexLen,
                 ball_mesh->index, GL_STATIC_DRAW);
    model->elementsCount = (size_t) ball_mesh->indexLen;
    free_mesh(ball_mesh);
    identity(model->modelMatrix);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    return model;
}