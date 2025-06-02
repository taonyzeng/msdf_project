#version 330

in vec2 texCoord;
out vec4 color;
uniform sampler2D msdf;
uniform vec4 bgColor;
uniform vec4 fgColor;

float outlineWidth = 0.05; // adjust based on your texture resolution
float haloWidth = 0.1;     // total width of halo effect

//uniform float pxRange; // set to distance field's pixel range

float screenPxRange() {
    vec2 unitRange = vec2(6.0)/vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max( 0.5*dot(unitRange, screenTexSize), 1.0 );
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
	vec4 msdf = texture( msdf, texCoord );
	float dist = median(msdf.r, msdf.g, msdf.b) - 0.5;
    float screenPxDistance = screenPxRange() * dist;
	
	// Glyph (inked area)
	float glyphAlpha = smoothstep(-outlineWidth, outlineWidth, 0.5 + screenPxDistance);

	// Halo around glyph
	float halo = smoothstep(haloWidth, outlineWidth, screenPxDistance);

	vec3 glyphColor = vec3(1.0);       // white glyph
	vec3 haloColor = vec3(1.0, 0.8, 0); // yellowish glow

	vec3 finalColor = mix(haloColor, glyphColor, glyphAlpha);
	float finalAlpha = max(glyphAlpha, halo * 0.5); // soften the halo

	color = vec4(finalColor, finalAlpha);
}