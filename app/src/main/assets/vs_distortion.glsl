attribute vec2 aPosition;
attribute float aVignette;
attribute vec2 aRedTextureCoord;
attribute vec2 aGreenTextureCoord;
attribute vec2 aBlueTextureCoord;
varying vec2 vRedTextureCoord;
varying vec2 vBlueTextureCoord;
varying vec2 vGreenTextureCoord;
varying float vVignette;
uniform float uTextureCoordScale;
void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vRedTextureCoord = aRedTextureCoord.xy * uTextureCoordScale;
    vGreenTextureCoord = aGreenTextureCoord.xy * uTextureCoordScale;
    vBlueTextureCoord = aBlueTextureCoord.xy * uTextureCoordScale;
    vVignette = aVignette;
}