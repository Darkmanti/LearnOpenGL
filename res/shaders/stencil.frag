#version 330 core

out vec4 result;

uniform vec4 outlineColor;

void main()
{
    result = outlineColor;
}