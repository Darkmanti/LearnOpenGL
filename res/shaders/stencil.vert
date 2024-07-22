#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 mvpMatrix;

uniform float scale;

void main()
{
    gl_Position = mvpMatrix * vec4(inPosition + (inNormal * scale), 1.0);
}