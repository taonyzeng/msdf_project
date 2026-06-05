#version 330

in vec2 texCoord;
out vec4 out_color;
uniform sampler2D msdf;
uniform vec4 bgColor;
uniform vec4 fgColor;

const float pxRange = 32.0; // set to distance field's pixel range

const float outlineDistance = 0.1f; // Between 0 and 0.5, 0 = thick outline, 0.5 = no outline
const vec4 outlineColor = vec4(0.0, 0.0, 0.0, 1.0); //black

float screenPxRange() {
    vec2 unitRange = vec2(6.0)/vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {

    float smoothing = 1.0f / pxRange;

    vec3 msd = texture(msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    float outlineFactor = smoothstep(0.5 - smoothing, 0.5 + smoothing, sd );

    vec4 color = mix(outlineColor, fgColor, outlineFactor);
    float alpha = smoothstep(outlineDistance - smoothing, outlineDistance + smoothing, sd );

	out_color = vec4( color.rgb, color.a * alpha );

}