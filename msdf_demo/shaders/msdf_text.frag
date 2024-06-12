#version 330

in vec2 texCoord;
out vec4 color;
uniform sampler2D msdf;
//uniform vec4 bgColor;
//uniform vec4 fgColor;

//uniform float pxRange; // set to distance field's pixel range

float screenPxRange() {
    vec2 unitRange = vec2(6.0)/vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec3 msd = texture(msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    //color = mix(bgColor, fgColor, opacity);

    color = vec4(0.80469, 0.917969, 0.9804688, opacity);
}