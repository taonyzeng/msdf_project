#version 330

in vec2 texCoord;
out vec4 color;

uniform sampler2D u_msdf;
uniform vec4 bgColor;   // Usually (0,0,0,0)
uniform vec4 fgColor;   // Text Color
uniform vec4 glowColor; // Glow Color (e.g., Red)
uniform float glowRange; // Glow width (Try 0.1 to 0.3)

float screenPxRange() {
    // Note: 6.0 matches your pxRange. 
    // If you change pxRange in your generator, change this too!
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

    // 1. SHARP TEXT LOGIC (Your original code)
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float textOpacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    // 2. SOFT GLOW LOGIC
    // We create a gradient that starts at 0.5 (edge) and fades out 
    // towards (0.5 - glowRange).
    float glowOpacity = smoothstep(0.5 - glowRange, 0.5, sd);

    // 3. COMPOSITION
    // First, blend the background with the glow
    vec4 backgroundWithGlow = mix(bgColor, glowColor, glowOpacity);
    
    // Then, blend the result with the sharp text on top
    color = mix(backgroundWithGlow, fgColor, textOpacity);
    
    // SAFETY: If the final pixel is completely transparent, 
    // discard to prevent invisible quad boxes from overlapping.
    if (color.a < 0.001) discard;
}