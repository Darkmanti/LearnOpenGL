#version 330 core

out vec4 result;

in vec2 TexCoords;

uniform sampler2D diffuseTexture;

void main()
{
    result = texture(diffuseTexture, TexCoords);
}