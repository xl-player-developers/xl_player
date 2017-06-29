attribute vec4 position;
attribute vec2 texcoord;
uniform float width_adjustment;
uniform mat4 modelMatrix;
varying vec2 tx;
void main(void){
    tx = vec2(texcoord.x * width_adjustment, texcoord.y);

    vec4 ballPos = modelMatrix * position;
    float j = asin(ballPos.y);
    float i = degrees(acos(ballPos.x / cos(j)));
    i -= 180.0;
    j = degrees(j);
    if(ballPos.z < 0.0){ i = -i; }
    float xx = i / 180.0 * 1.1;
    float yy = j / 90.0 * 1.1;
    gl_Position = vec4(xx, yy, 0.1, 1.0);
}