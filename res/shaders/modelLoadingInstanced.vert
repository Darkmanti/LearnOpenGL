#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 2) in vec2 inTexCoords;

layout (location = 3) in mat4 instanceMatrix;

out VS_OUT
{
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vs_out.TexCoords = inTexCoords;
    gl_Position = projection * view * instanceMatrix * vec4(inPosition, 1.0);
}