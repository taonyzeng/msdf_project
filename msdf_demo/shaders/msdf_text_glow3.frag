#version 330

in vec2 texCoord;
out vec4 color;

uniform sampler2D u_msdf;
uniform vec4 fgColor;   // Text Color
uniform vec4 glowColor; // Glow Color
uniform float glowRange; // Glow width (0.1 to 0.4)

float screenPxRange() {
    // Note: 6.0 matches your pxRange. 
    vec2 unitRange = vec2(32.0)/vec2(textureSize(u_msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec3 msd = texture(u_msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    // 1. Sharp Text Alpha (using your screenPxRange logic)
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float textAlpha = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    // 2. Soft Glow Alpha
    // Fades from 1.0 at the edge (0.5) down to 0.0 at (0.5 - glowRange)
    float glowAlpha = smoothstep(0.5 - glowRange, 0.5, sd);

    // 3. Color Blending
    // We mix the RGB colors based on textAlpha. 
    // If we're inside the letter, we see fgColor. 
    // If we're just outside in the glow zone, we see glowColor.
    vec3 finalRGB = mix(glowColor.rgb, fgColor.rgb, textAlpha);

    // 4. Final Alpha Calculation
    // We want the highest alpha value between the text and the glow.
    // This ensures the glow is visible behind/around the text.
    float finalAlpha = max(textAlpha * fgColor.a, glowAlpha * glowColor.a);

    // 5. Cleanup
    // If there's no text and no glow, don't bother rendering the pixel.
    if (finalAlpha < 0.001) discard;

    color = vec4(finalRGB, finalAlpha);
}