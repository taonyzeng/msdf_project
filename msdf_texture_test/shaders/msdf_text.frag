#version 330

in vec2 texCoord;
out vec4 color;
uniform sampler2D u_msdf;
//uniform vec4 bg_clr;
uniform vec4 fg_clr;

//const vec4 fg_clr = vec4( 0.0f, 1.0f, 1.0f, 1.0f );

//uniform float pxRange; // set to distance field's pixel range

float screenPxRange() {
    vec2 unitRange = vec2(6.0)/vec2(textureSize(u_msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec3 msd = texture(u_msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    //color = mix(fg_clr, bgColor, opacity);
    color = fg_clr * opacity;
    //color = vec4(0.80469, 0.917969, 0.9804688, opacity);
}