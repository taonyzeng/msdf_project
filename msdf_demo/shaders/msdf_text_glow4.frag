#version 330

#define TIME_SPEED 0.5

uniform sampler2D u_msdf;
uniform float time;

in vec2 texCoord;

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

// Must match the pxRange used when generating the MSDF atlas texture
const float MSDF_PX_RANGE = 6.0;

float screenPxRange() {
	vec2 unitRange = vec2(MSDF_PX_RANGE) / vec2(textureSize(u_msdf, 0));
	//vec2 unitRange = vec2(MSDF_PX_RANGE) / vec2(48, 48);
	vec2 screenTexSize = vec2(1.0) / fwidth(texCoord);
	return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

const vec4 fgColor = vec4(0.0, 1.0, 1.0, 1.0);
const vec4 outlineColor = vec4(0.0, 0.0, 0.0, 1.0);

// Ranges:
// -0.3 < thickness < 0.3
// 0.0 < softness < 0.5
float thickness = -0.2;
float outlineThickness = 0.4;
float softness = 0.05;
float outlineSoftness = 0.5;

void main() {
	vec4 texel = texture(u_msdf, texCoord);
	//float a = texel.a;
	float dist = median(texel.r, texel.g, texel.b);
	if (dist <= 0.001) {
		discard;
	}
	float pxRange = screenPxRange();
	dist -= 0.5;
	
	dist += thickness;

	float bodySoftnessPx = softness * pxRange;
  	float bodyPxDist = pxRange * dist;
	float bodyOpacity = smoothstep(-0.5 - bodySoftnessPx, 0.5 + bodySoftnessPx, bodyPxDist);
	
	float outlineSoftnessPx = outlineSoftness * pxRange;
	float charPxDist = pxRange * (dist + outlineThickness);
	float charOpacity = smoothstep(-0.5 - outlineSoftnessPx, 0.5 + outlineSoftnessPx, charPxDist);

	float outlineOpacity = charOpacity - bodyOpacity;
	
	vec3 color = mix(outlineColor.rgb, fgColor.rgb, bodyOpacity);
	float alpha = bodyOpacity * fgColor.a + outlineOpacity * outlineColor.a;
	
	gl_FragColor = vec4(color, alpha);
}