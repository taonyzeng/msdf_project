#version 330

in vec2 texCoord;
out vec4 color;

uniform sampler2D msdf;
uniform vec4 bgColor; // unused for transparent background


float screenPxRange() {
    vec2 unitRange = vec2(6.0) / vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec3 msd = texture(msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);

    // Main glyph opacity (white)
    float glyphAlpha = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    // Halo effect opacity (soft glow outside glyph)
    float haloEdge = 2.5; // controls how far the halo spreads
    float haloAlpha = clamp(1.0 - abs(screenPxDistance) / haloEdge, 0.0, 1.0);
    haloAlpha *= (1.0 - glyphAlpha); // mask so halo doesn't overwrite glyph
	
	vec4 fgColor = vec4(1.0, 1.0, 1.0, 1.0); // typically white
	vec4 haloColor = vec4(1.0, 0.8, 0.0, 1.0); 
	
    // Final color blend
    vec4 glyph = fgColor * glyphAlpha;
    vec4 halo = haloColor * haloAlpha;

    color = glyph + halo; // additive blend
}