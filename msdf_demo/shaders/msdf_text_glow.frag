#version 300 es
precision mediump float;

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D u_msdf;
uniform vec4 u_textColor;
uniform vec4 u_glowColor;

// Controls how far the glow extends (e.g., 0.2 is a decent soft glow)
uniform float u_glowRange; 
// Controls the "thickness" of the glow base (0.0 is usually best)
uniform float u_glowOffset; 

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    // 1. Sample MSDF and get distance
    vec3 msd = texture(u_msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    // 2. Standard Anti-aliasing for the sharp text
    float w = fwidth(sd);
    float textAlpha = smoothstep(0.5 - w, 0.5 + w, sd);

    // 3. Soft Glow Logic
    // Instead of a tiny width 'w', we use u_glowRange.
    // This creates a smooth gradient from the edge (0.5) outward.
    float glowThreshold = 0.5 - u_glowOffset;
    float glowAlpha = smoothstep(glowThreshold - u_glowRange, glowThreshold, sd);

    // 4. Layering
    // If we are inside the text, use text color. 
    // If we are in the glow area, use glow color.
    vec4 color = mix(u_glowColor, u_textColor, textAlpha);
    
    // The final alpha is the stronger of the two effects.
    // This ensures the text stays sharp on top of the soft glow.
    float alpha = max(textAlpha, glowAlpha * u_glowColor.a);
    
    // 5. Background Cleanup
    if (alpha < 0.001) {
        discard;
    }

    fragColor = vec4(color.rgb, alpha);
}