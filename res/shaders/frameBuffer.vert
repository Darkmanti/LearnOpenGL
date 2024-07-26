#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(inPosition.x, inPosition.y, 0.0f, 1.0f);
    TexCoords = inTexCoords;
}