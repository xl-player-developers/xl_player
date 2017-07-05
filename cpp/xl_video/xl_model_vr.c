//
// Created by gutou on 2017/6/6.
//

#include <android/asset_manager.h>
#include "xl_glsl_program.h"

#include "xl_model_vr.h"
#include "xl_mat4.h"
#include "xl_texture.h"
#include "xl_mesh_factory.h"

static AAssetManager *pManager;
static GLfloat _eyez;

static void updateViewMatrix(xl_model * model) {
    lookAt(-0.012f, 0, _eyez, 0, 0, -1.0f, 0, 1, 0, model->view_matrix);
    lookAt(0.012f, 0, _eyez, 0, 0, -1.0f, 0, 1, 0, model->view_matrix_r);
}

static void updateEyePosition(xl_model * model, GLfloat eyez) {
    if (eyez != _eyez) {
        _eyez = eyez;
        updateViewMatrix(model);
    }
}

static void updateProjectionMatrix(xl_model *model) {
    perspective(60, (float)model->viewport_w / (float)model->viewport_h / 2.0f, 0.01f, 100, model->projectionMatrix);
}

void resetFrameBufferSize(xl_model *model){
    glBindTexture(GL_TEXTURE_2D, model->frame_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, model->viewport_w, model->viewport_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindRenderbuffer(GL_RENDERBUFFER, model->color_render_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, model->viewport_w, model->viewport_h);
}

static void resize(xl_model *model, int w, int h) {
    model->viewport_w = w;
    model->viewport_h = h;
    resetFrameBufferSize(model);
    updateProjectionMatrix(model);
}

static void update_frame_vr(xl_model *model, AVFrame * frame){
    if(model->pixel_format != frame->format){
        model->pixel_format = (enum AVPixelFormat)frame->format;
        model->program = xl_glsl_program_get(model->type, model->pixel_format, pManager);
        switch(frame->format){
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
    }
    model->width_adjustment = (float)frame->width / (float)frame->linesize[0];
    model->updateTexture(model, frame);
}


static inline void draw_left(xl_model *model){
    glUniformMatrix4fv(model->program->viewMatrixLoc, 1, GL_FALSE, model->view_matrix);
    glViewport(0, 0, model->viewport_w / 2, model->viewport_h);
    glDrawElements(GL_TRIANGLES, (GLsizei) model->elementsCount, GL_UNSIGNED_INT, 0);
}

static inline void draw_right(xl_model * model){
    glUniformMatrix4fv(model->program->viewMatrixLoc, 1, GL_FALSE, model->view_matrix_r);
    glViewport(model->viewport_w / 2, 0, model->viewport_w / 2, model->viewport_h);
    glDrawElements(GL_TRIANGLES, (GLsizei) model->elementsCount, GL_UNSIGNED_INT, 0);
}

static void draw_distortion(xl_model * model, xl_eye * eye){
    xl_glsl_program_distortion * program = model->program_distortion;
    glUseProgram(program->program);
    glBindBuffer(GL_ARRAY_BUFFER, eye->point_buffer_id);
    glVertexAttribPointer((GLuint) program->positon_location, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(0 * sizeof(float)));
    glEnableVertexAttribArray((GLuint) program->positon_location);
    glVertexAttribPointer((GLuint) program->vignette_location, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray((GLuint) program->vignette_location);

    glVertexAttribPointer((GLuint) program->texcoord_r_location, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray((GLuint) program->texcoord_r_location);
    glVertexAttribPointer((GLuint) program->texcoord_g_location, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray((GLuint) program->texcoord_g_location);
    glVertexAttribPointer((GLuint) program->texcoord_b_location, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray((GLuint) program->texcoord_b_location);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, model->frame_texture_id);
    glUniform1i(program->tex, 0);

    float _resolutionScale = 1;
    glUniform1f(program->texcoord_scale_location, _resolutionScale);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eye->index_buffer_id);
    glDrawElements(GL_TRIANGLES, (GLsizei) eye->index_count, GL_UNSIGNED_INT, 0);
}


static void draw(xl_model *model){
    xl_glsl_program * program = model->program;
    glBindFramebuffer(GL_FRAMEBUFFER, model->frame_buffer_id);
    // draw to framebuffer
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, model->viewport_w / 2, model->viewport_h);
    glUseProgram(program->program);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[0]);
    glVertexAttribPointer((GLuint) program->positon_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray((GLuint) program->positon_location);
    glBindBuffer(GL_ARRAY_BUFFER, model->vbos[1]);
    glVertexAttribPointer((GLuint) program->texcoord_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray((GLuint) program->texcoord_location);
    model->bind_texture(model);
    glUniform1f(program->linesize_adjustment_location, model->width_adjustment);
    glUniformMatrix4fv(program->modelMatrixLoc, 1, GL_FALSE, model->modelMatrix);
    glUniformMatrix4fv(program->projectionMatrixLoc, 1, GL_FALSE, model->projectionMatrix);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vbos[2]);
    draw_left(model);
    draw_right(model);

    //draw to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, model->viewport_w, model->viewport_h);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, model->viewport_w / 2, model->viewport_h);
    draw_distortion(model, &model->left_eye);
    glScissor(model->viewport_w / 2, 0, model->viewport_w / 2, model->viewport_h);
    draw_distortion(model, &model->right_eye);
    glDisable(GL_SCISSOR_TEST);
}

static void updateHead(xl_model *model, GLfloat *headMatrix) {
    for (int i = 0; i < 16; ++i) {
        model->modelMatrix[i] = headMatrix[i];
    }
}

static void update_distance_vr(xl_model *model, GLfloat distance){
    distance = distance > 2 ? 2 : (distance < 0.5f ? 0.5f : distance);
    distance = 1 / distance;
    updateEyePosition(model, distance - 1.0f);
}

static void setup_frame_buffer(xl_model *model){
    glGenTextures(1, &model->frame_texture_id);
    glBindTexture(GL_TEXTURE_2D, model->frame_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenRenderbuffers(1, &model->color_render_id);
    glBindRenderbuffer(GL_RENDERBUFFER, model->color_render_id);
    glGenFramebuffers(1, &model->frame_buffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, model->frame_buffer_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, model->frame_texture_id, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, model->color_render_id);
}

static void setup_eye(xl_eye *eye, xl_mesh * eye_mesh){
    glGenBuffers(1, &eye->point_buffer_id);
    glGenBuffers(1, &eye->index_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, eye->point_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * eye_mesh->ppLen, eye_mesh->pp, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eye->index_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * eye_mesh->indexLen, eye_mesh->index, GL_STATIC_DRAW);
    eye->index_count = (size_t) eye_mesh->indexLen;
}

xl_model * model_vr_create(AAssetManager *pAAssetManager){
    pManager = pAAssetManager;
    xl_model *model = (xl_model *) malloc(sizeof(xl_model));
    model->type = VR;
    model->program = NULL;
    model->draw = NULL;
    model->updateTexture = NULL;
    model->pixel_format = AV_PIX_FMT_NONE;
    model->resize = resize;
    model->update_frame = update_frame_vr;
    model->draw = draw;
    model->updateModelRotation = NULL;
    model->update_distance = update_distance_vr;
    model->updateHead = updateHead;
    xl_mesh * ball = get_ball_mesh();
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

    xl_mesh * left_eye_mesh = get_distortion_mesh(0);
    setup_eye(&model->left_eye, left_eye_mesh);
    free_mesh(left_eye_mesh);
    xl_mesh * right_eye_mesh = get_distortion_mesh(1);
    setup_eye(&model->right_eye, right_eye_mesh);
    free_mesh(right_eye_mesh);
    setup_frame_buffer(model);
    model->program_distortion = xl_glsl_program_distortion_get(pAAssetManager);

    identity(model->modelMatrix);
    _eyez = 0;
    updateViewMatrix(model);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    return model;
}

void model_vr_free(xl_model *  model){
    if(model->type != VR) return;
    glDeleteBuffers(1, &model->left_eye.point_buffer_id);
    glDeleteBuffers(1, &model->left_eye.index_buffer_id);
    glDeleteBuffers(1, &model->right_eye.point_buffer_id);
    glDeleteBuffers(1, &model->right_eye.index_buffer_id);
    glDeleteFramebuffers(1, &model->frame_buffer_id);
    glDeleteRenderbuffers(1, &model->color_render_id);
    glDeleteTextures(1, &model->frame_texture_id);
}