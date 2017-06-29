//
// Created by gutou on 2017/5/5.
//

#include "xl_glsl_program.h"

static AAssetManager *pAAssetManager;

static void init_rect_nv12();
static void init_rect_yuv_420p();
static void init_rect_oes();
static void init_ball_nv12();
static void init_ball_yuv_420p();
static void init_ball_oes();
static void init_expand_nv12();
static void init_expand_yuv_420p();
static void init_expand_oes();

static xl_glsl_program rect_nv12 = {
        .has_init = 0,
        .init = init_rect_nv12
};
static xl_glsl_program rect_yuv_420p = {
        .has_init = 0,
        .init = init_rect_yuv_420p
};
static xl_glsl_program rect_oes = {
        .has_init = 0,
        .init = init_rect_oes
};
static xl_glsl_program ball_nv12 = {
        .has_init = 0,
        .init = init_ball_nv12
};
static xl_glsl_program ball_yuv_420p = {
        .has_init = 0,
        .init = init_ball_yuv_420p
};
static xl_glsl_program ball_oes = {
        .has_init = 0,
        .init = init_ball_oes
};
static xl_glsl_program expand_nv12 = {
        .has_init = 0,
        .init = init_expand_nv12
};
static xl_glsl_program expand_yuv_420p = {
        .has_init = 0,
        .init = init_expand_yuv_420p
};
static xl_glsl_program expand_oes = {
        .has_init = 0,
        .init = init_expand_oes
};
static xl_glsl_program_distortion distortion = {
        .has_init = 0
};

static char * readFile(const char * fname, AAssetManager * manager){
    AAsset * pAAsset = NULL;
    char * pBuffer = NULL;
    size_t size;
    int numBytes;
    if(NULL == manager){
        LOGE("AAssetManager null!");
        return NULL;
    }
    pAAsset = AAssetManager_open(manager, fname, AASSET_MODE_UNKNOWN);
    size = (size_t)AAsset_getLength(pAAsset);
    pBuffer = (char *)malloc(size + 1);
    pBuffer[size] = '\0';
    numBytes = AAsset_read(pAAsset, pBuffer, size);
    LOGI("read file %s, numbytes %d", fname, numBytes);
    AAsset_close(pAAsset);
    return pBuffer;
}

static GLuint loadShader(GLenum shaderType, const char * shaderSrc){
    GLuint shader;
    GLint compiled;
    shader = glCreateShader(shaderType);
    if(shader == 0) return 0;
    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled){
        GLint infoLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 0){
            char * infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOGE("compile shader error ==>\n%s\n\n%s\n", shaderSrc, infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}


static GLuint loadProgram(const char * vsFile, const char * fsFile, AAssetManager * pAAssetManager){
    char * vsSrc = readFile(vsFile, pAAssetManager);
    char * fsSrc = readFile(fsFile, pAAssetManager);
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fsSrc);
    free(vsSrc);
    free(fsSrc);
    if(vertexShader == 0 || fragmentShader == 0) return 0;
    GLint linked;
    GLuint pro = glCreateProgram();
    if(pro == 0){
        LOGE("create program error!");
    }
    glAttachShader(pro, vertexShader);
    glAttachShader(pro, fragmentShader);
    glLinkProgram(pro);
    glGetProgramiv(pro, GL_LINK_STATUS, &linked);
    if(!linked){
        GLint infoLen;
        glGetProgramiv(pro, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 0){
            char * infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(pro, infoLen, NULL, infoLog);
            LOGE("link program error ==>\n%s\n", infoLog);
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(pro);
        return 0;
    }
    return pro;
}

static void init_rect_nv12(){
    GLuint pro = loadProgram("vs_es2_rect.glsl", "fs_es2_nv12.glsl", pAAssetManager);
    rect_nv12.program = pro;
    rect_nv12.positon_location = glGetAttribLocation(pro, "position");
    rect_nv12.texcoord_location = glGetAttribLocation(pro, "texcoord");
    rect_nv12.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    rect_nv12.x_scale_location = glGetUniformLocation(pro, "x_scale");
    rect_nv12.y_scale_location = glGetUniformLocation(pro, "y_scale");
    rect_nv12.frame_rotation_location = glGetUniformLocation(pro, "frame_rotation");
    rect_nv12.tex_y = glGetUniformLocation(pro, "tex_y");
    rect_nv12.tex_u = glGetUniformLocation(pro, "tex_u");
}

static void init_rect_yuv_420p(){
    GLuint pro = loadProgram("vs_es2_rect.glsl", "fs_es2_yuv_420p.glsl", pAAssetManager);
    rect_yuv_420p.program = pro;
    rect_yuv_420p.positon_location = glGetAttribLocation(pro, "position");
    rect_yuv_420p.texcoord_location = glGetAttribLocation(pro, "texcoord");
    rect_yuv_420p.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    rect_yuv_420p.x_scale_location = glGetUniformLocation(pro, "x_scale");
    rect_yuv_420p.y_scale_location = glGetUniformLocation(pro, "y_scale");
    rect_yuv_420p.frame_rotation_location = glGetUniformLocation(pro, "frame_rotation");
    rect_yuv_420p.tex_y = glGetUniformLocation(pro, "tex_y");
    rect_yuv_420p.tex_u = glGetUniformLocation(pro, "tex_u");
    rect_yuv_420p.tex_v = glGetUniformLocation(pro, "tex_v");
}

static void init_rect_oes(){
    GLuint pro = loadProgram("vs_es2_rect.glsl", "fs_es2_egl_ext.glsl", pAAssetManager);
    rect_oes.program = pro;
    rect_oes.positon_location = glGetAttribLocation(pro, "position");
    rect_oes.texcoord_location = glGetAttribLocation(pro, "texcoord");
    rect_oes.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    rect_oes.x_scale_location = glGetUniformLocation(pro, "x_scale");
    rect_oes.y_scale_location = glGetUniformLocation(pro, "y_scale");
    rect_oes.frame_rotation_location = glGetUniformLocation(pro, "frame_rotation");
    rect_oes.tex_y = glGetUniformLocation(pro, "tex_y");
    rect_oes.texture_matrix_location = glGetUniformLocation(pro, "tx_matrix");
}

static void init_ball_nv12(){
    GLuint pro = loadProgram("vs_es2_ball.glsl", "fs_es2_nv12.glsl", pAAssetManager);
    ball_nv12.program = pro;
    ball_nv12.positon_location = glGetAttribLocation(pro, "position");
    ball_nv12.texcoord_location = glGetAttribLocation(pro, "texcoord");
    ball_nv12.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    ball_nv12.modelMatrixLoc = glGetUniformLocation(pro, "modelMatrix");
    ball_nv12.viewMatrixLoc = glGetUniformLocation(pro, "viewMatrix");
    ball_nv12.projectionMatrixLoc = glGetUniformLocation(pro, "projectionMatrix");
    ball_nv12.tex_y = glGetUniformLocation(pro, "tex_y");
    ball_nv12.tex_u = glGetUniformLocation(pro, "tex_u");
}

static void init_ball_yuv_420p() {
    GLuint pro = loadProgram("vs_es2_ball.glsl", "fs_es2_yuv_420p.glsl", pAAssetManager);
    ball_yuv_420p.program = pro;
    ball_yuv_420p.positon_location = glGetAttribLocation(pro, "position");
    ball_yuv_420p.texcoord_location = glGetAttribLocation(pro, "texcoord");
    ball_yuv_420p.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    ball_yuv_420p.modelMatrixLoc = glGetUniformLocation(pro, "modelMatrix");
    ball_yuv_420p.viewMatrixLoc = glGetUniformLocation(pro, "viewMatrix");
    ball_yuv_420p.projectionMatrixLoc = glGetUniformLocation(pro, "projectionMatrix");
    ball_yuv_420p.tex_y = glGetUniformLocation(pro, "tex_y");
    ball_yuv_420p.tex_u = glGetUniformLocation(pro, "tex_u");
    ball_yuv_420p.tex_v = glGetUniformLocation(pro, "tex_v");
}

static void init_ball_oes(){
    GLuint pro = loadProgram("vs_es2_ball.glsl", "fs_es2_egl_ext.glsl", pAAssetManager);
    ball_oes.program = pro;
    ball_oes.positon_location = glGetAttribLocation(pro, "position");
    ball_oes.texcoord_location = glGetAttribLocation(pro, "texcoord");
    ball_oes.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    ball_oes.modelMatrixLoc = glGetUniformLocation(pro, "modelMatrix");
    ball_oes.viewMatrixLoc = glGetUniformLocation(pro, "viewMatrix");
    ball_oes.projectionMatrixLoc = glGetUniformLocation(pro, "projectionMatrix");
    ball_oes.tex_y = glGetUniformLocation(pro, "tex_y");
    ball_oes.texture_matrix_location = glGetUniformLocation(pro, "tx_matrix");
}

static void init_expand_nv12(){
    GLuint pro = loadProgram("vs_es2_expand.glsl", "fs_es2_nv12.glsl", pAAssetManager);
    expand_nv12.program = pro;
    expand_nv12.positon_location = glGetAttribLocation(pro, "position");
    expand_nv12.texcoord_location = glGetAttribLocation(pro, "texcoord");
    expand_nv12.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    expand_nv12.modelMatrixLoc = glGetUniformLocation(pro, "modelMatrix");
    expand_nv12.tex_y = glGetUniformLocation(pro, "tex_y");
    expand_nv12.tex_u = glGetUniformLocation(pro, "tex_u");
}

static void init_expand_yuv_420p(){
    GLuint pro = loadProgram("vs_es2_expand.glsl", "fs_es2_yuv_420p.glsl", pAAssetManager);
    expand_yuv_420p.program = pro;
    expand_yuv_420p.positon_location = glGetAttribLocation(pro, "position");
    expand_yuv_420p.texcoord_location = glGetAttribLocation(pro, "texcoord");
    expand_yuv_420p.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    expand_yuv_420p.modelMatrixLoc = glGetUniformLocation(pro, "modelMatrix");
    expand_yuv_420p.tex_y = glGetUniformLocation(pro, "tex_y");
    expand_yuv_420p.tex_u = glGetUniformLocation(pro, "tex_u");
    expand_yuv_420p.tex_v = glGetUniformLocation(pro, "tex_v");
}

static void init_expand_oes(){
    GLuint pro = loadProgram("vs_es2_expand.glsl", "fs_es2_egl_ext.glsl", pAAssetManager);
    expand_oes.program = pro;
    expand_oes.positon_location = glGetAttribLocation(pro, "position");
    expand_oes.texcoord_location = glGetAttribLocation(pro, "texcoord");
    expand_oes.linesize_adjustment_location = glGetUniformLocation(pro, "width_adjustment");
    expand_oes.modelMatrixLoc = glGetUniformLocation(pro, "modelMatrix");
    expand_oes.tex_y = glGetUniformLocation(pro, "tex_y");
    expand_oes.texture_matrix_location = glGetUniformLocation(pro, "tx_matrix");
}

xl_glsl_program * xl_glsl_program_get(ModelType type, int pixel_format, AAssetManager *pManager){
    pAAssetManager = pManager;
    xl_glsl_program * pro = NULL;
    switch(type){
        case Rect:
            switch(pixel_format){
                case AV_PIX_FMT_NV12:
                    pro = &rect_nv12;
                    break;
                case AV_PIX_FMT_YUV420P:
                    pro = &rect_yuv_420p;
                    break;
                case XL_PIX_FMT_EGL_EXT:
                    pro = &rect_oes;
                    break;
                default:
                    break;
            }
            break;
        case Ball:
        case VR:
        case Planet:
        case Architecture:
            switch(pixel_format){
                case AV_PIX_FMT_NV12:
                    pro = &ball_nv12;
                    break;
                case AV_PIX_FMT_YUV420P:
                    pro = &ball_yuv_420p;
                    break;
                case XL_PIX_FMT_EGL_EXT:
                    pro = &ball_oes;
                    break;
                default:
                    break;
            }
            break;
        case Expand:
            switch(pixel_format){
                case AV_PIX_FMT_NV12:
                    pro = &expand_nv12;
                    break;
                case AV_PIX_FMT_YUV420P:
                    pro = &expand_yuv_420p;
                    break;
                case XL_PIX_FMT_EGL_EXT:
                    pro = &expand_oes;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    if(pro != NULL && pro->has_init == 0){
        pro->init();
        pro->has_init = 1;
    }
    return pro;
}

xl_glsl_program_distortion * xl_glsl_program_distortion_get(AAssetManager *pManager){
    if(distortion.has_init == 0){
        GLuint pro = loadProgram("vs_distortion.glsl", "fs_distortion.glsl", pManager);
        distortion.program = pro;
        distortion.positon_location = glGetAttribLocation(pro, "aPosition");
        distortion.vignette_location = glGetAttribLocation(pro, "aVignette");
        distortion.texcoord_r_location = glGetAttribLocation(pro, "aRedTextureCoord");
        distortion.texcoord_g_location = glGetAttribLocation(pro, "aGreenTextureCoord");
        distortion.texcoord_b_location = glGetAttribLocation(pro, "aBlueTextureCoord");
        distortion.texcoord_scale_location = glGetUniformLocation(pro, "uTextureCoordScale");
        distortion.tex = glGetUniformLocation(pro, "uTextureSampler");
    }
    return &distortion;
}

void xl_glsl_program_clear_all(){
    glDeleteProgram(rect_nv12.program);
    rect_nv12.has_init = 0;

    glDeleteProgram(rect_yuv_420p.program);
    rect_yuv_420p.has_init = 0;

    glDeleteProgram(rect_oes.program);
    rect_oes.has_init = 0;

    glDeleteProgram(ball_nv12.program);
    ball_nv12.has_init = 0;

    glDeleteProgram(ball_yuv_420p.program);
    ball_yuv_420p.has_init = 0;

    glDeleteProgram(ball_oes.program);
    ball_oes.has_init = 0;

    glDeleteProgram(expand_nv12.program);
    expand_nv12.has_init = 0;

    glDeleteProgram(expand_yuv_420p.program);
    expand_yuv_420p.has_init = 0;

    glDeleteProgram(expand_oes.program);
    expand_oes.has_init = 0;

    glDeleteProgram(distortion.program);
    distortion.has_init = 0;
}