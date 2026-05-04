#version 330 core
precision mediump float;

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D u_msdf;
uniform vec4 u_textColor;
uniform vec4 u_haloColor;
uniform float u_haloWidth; // 0.0 to 0.5

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    // 1. Sample MSDF and get distance
    vec3 msd = texture(u_msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    // 2. Calculate smooth edges (Anti-aliasing)
    // We add a tiny epsilon (0.001) to prevent division by zero in flat areas
    float w = max(fwidth(sd), 0.001);

    // 3. Calculate Alphas
    // Core: The main text (standard edge at 0.5)
    float coreAlpha = smoothstep(0.5 - w, 0.5 + w, sd);
    
    // Halo: The expanded shape (edge shifted by u_haloWidth)
    // Use u_haloWidth = 0.0 for no halo.
    float haloAlpha = smoothstep(0.5 - u_haloWidth - w, 0.5 - u_haloWidth + w, sd);

    // 4. Manual Composition
    // We start with the halo color and layer the text color on top of it.
    vec4 finalColor = mix(u_haloColor, u_textColor, coreAlpha);

    // Apply the total shape mask (haloAlpha defines the outermost boundary)
    finalColor.a *= haloAlpha;

    // --- THE FIXES ---
    
    // A. Discard pixels that are essentially transparent. 
    // This fixes the "solid background" issue even if Blending is disabled.
    if (finalColor.a < 0.01) {
        discard;
    }

    fragColor = finalColor;
}