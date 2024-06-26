#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    // linearly interpolate between both textures (80% container, 20% awesomeface)
    //FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2);
    FragColor = texture(texture1, texCoord);
}