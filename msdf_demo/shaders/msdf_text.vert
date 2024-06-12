#version 330

uniform mat4 MVP;

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 texCoord;
 
out vec2 uvCoord;

void main() {
	uvCoord = vec2(1-texCoord.x, texCoord.y);
	gl_Position = MVP * vec4(vPos, 0.0, 1.0);
}