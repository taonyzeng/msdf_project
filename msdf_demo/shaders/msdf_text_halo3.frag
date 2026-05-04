#version 330 core
// OR #version 330 core if on desktop OpenGL
precision mediump float;

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D u_msdf;
// The main text color
uniform vec4 u_textColor;

// NEW: The color of the halo outline
uniform vec4 u_haloColor;
// NEW: Width of halo relative to SDF distance.
// Try values between 0.0 (no halo) to around 0.2 (thick outline).
uniform float u_haloWidth;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    // 1. Sample and calculate signed distance
    vec3 msd = texture(u_msdf, texCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    // 2. Determine anti-aliasing width
    // fwidth tells us how fast the distance changes between screen pixels.
    // We use this to create a smooth transition at the edge.
    // 0.5 is a standard sharpening factor. Increase slightly for sharper, decrease for softer.
    float w = fwidth(sd) * 0.5;

    // 3. Calculate Core Alpha (The main text body)
    // Standard MSDF edge is at 0.5. We smoothstep around it by width 'w'.
    float coreAlpha = smoothstep(0.5 - w, 0.5 + w, sd);

    // 4. Calculate Halo Alpha (The outer ring)
    // The outer edge of the halo is pushed out by u_haloWidth.
    // The new boundary is at (0.5 - u_haloWidth).
    float haloBoundary = 0.5 - u_haloWidth;
    float haloAlpha = smoothstep(haloBoundary - w, haloBoundary + w, sd);

    // 5. Composition
    // We want the text color where the core is active.
    // We want the halo color where the halo is active BUT the core is not.

    // Start by mixing the halo color and text color based on the core Alpha.
    // If inside core, use text color. If outside core (but inside halo), use halo color.
    vec4 compositeColor = mix(u_haloColor, u_textColor, coreAlpha);

    // Apply the overall shape mask. The total visible shape is defined by the haloAlpha.
    compositeColor.a *= haloAlpha;
        
    // Final opacity adjustment if the input uniform colors have transparency
    // (Optional, depends on how you handle blending in your host application)
    // compositeColor.a *= max(u_textColor.a, u_haloColor.a);

    if (compositeColor.a < 0.001) {
            discard;
        }

    fragColor = compositeColor;
}