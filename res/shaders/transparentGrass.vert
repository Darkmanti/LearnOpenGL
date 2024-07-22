#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;

out vec2 TexCoords;

uniform mat4 mvpMatrix;

void main()
{
    TexCoords = inTexCoords;
    gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}