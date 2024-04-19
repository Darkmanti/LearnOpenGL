#version 330 core

out vec4 result;

in vec2 texCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
    vec4 mixedTexture = mix(texture(texture0, texCoord), texture(texture1, texCoord), 0.5);
    result = mixedTexture;
}