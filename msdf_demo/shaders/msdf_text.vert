#version 330

uniform mat4 MVP;

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 uvCoord;
 
out vec2 texCoord;

void main() {
	texCoord = vec2(uvCoord.x, uvCoord.y);
	gl_Position = vec4(vPos, 0.0, 1.0);
}