#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

out VS_OUT
{
    vec2 TexCoords;
} vs_out;

uniform mat4 mvpMatrix;

void main()
{
    vs_out.TexCoords = inTexCoords;
    gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}