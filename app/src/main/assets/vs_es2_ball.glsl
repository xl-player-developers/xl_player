attribute vec3 position;
attribute vec2 texcoord;
uniform float width_adjustment;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
varying vec2 tx;

void main(){
    tx = vec2(texcoord.x * width_adjustment, texcoord.y);
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}