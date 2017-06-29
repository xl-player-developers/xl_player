attribute vec3 position;
attribute vec2 texcoord;
uniform float width_adjustment;
uniform float x_scale;
uniform float y_scale;
uniform int frame_rotation;
varying vec2 tx;

const mat2 rotation90 = mat2(
    0.0, -1.0,
    1.0, 0.0
);
const mat2 rotation180 = mat2(
    -1.0, 0.0,
    0.0, -1.0
);
const mat2 rotation270 = mat2(
    0.0, 1.0,
    -1.0, 0.0
);
void main(){
    tx = vec2(texcoord.x * width_adjustment, texcoord.y);
    vec2 xy = vec2(position.x * x_scale, position.y * y_scale);
    if(frame_rotation == 1){
        xy = rotation90 * xy;
    }else if(frame_rotation == 2){
        xy = rotation180 * xy;
    }else if(frame_rotation == 3){
        xy = rotation270 * xy;
    }
    gl_Position = vec4(xy, position.z, 1.0);
}