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

    const float glyphEdge = 0.6f; // push edge inward, "shrinking" the fill slightly

    float smoothing = 1.0f / pxRange;
    float softness = 0.25f; // 0 = hard uniform outline, > 0 = fades outward (max ~0.5 - outlineDistance)

    vec3 msd = texture(msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    // 1 inside glyph body, 0 outside
    float fillMask = smoothstep(glyphEdge - smoothing, glyphEdge + smoothing, sd);

    // Outer edge: sharp when softness=0 (just AA smoothing), fades inward as softness increases
    float outerEdge = smoothstep(outlineDistance - smoothing, outlineDistance + smoothing + softness, sd);

    // Outline ring = visible outer area minus glyph fill
    float outlineFade = outerEdge * (1.0 - fillMask);

    vec3 rgb = outlineColor.rgb * outlineFade + fgColor.rgb * fillMask;
    float a   = outlineColor.a  * outlineFade + fgColor.a  * fillMask;

    out_color = vec4(rgb, a);

}