#extension GL_OES_EGL_image_external : require
precision mediump float;
uniform mat4 tx_matrix;
uniform samplerExternalOES tex_y;
varying vec2 tx;

void main(){
    vec2 tx_transformed = (tx_matrix * vec4(tx, 0, 1.0)).xy;
    gl_FragColor = texture2D(tex_y, tx_transformed);
}