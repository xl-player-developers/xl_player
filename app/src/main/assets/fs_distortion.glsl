precision mediump float;
varying vec2 vRedTextureCoord;
varying vec2 vBlueTextureCoord;
varying vec2 vGreenTextureCoord;
varying float vVignette;
uniform sampler2D uTextureSampler;
void main() {
    gl_FragColor = vVignette * vec4(texture2D(uTextureSampler, vRedTextureCoord).r,
                                    texture2D(uTextureSampler, vGreenTextureCoord).g,
                                    texture2D(uTextureSampler, vBlueTextureCoord).b, 1.0);
}