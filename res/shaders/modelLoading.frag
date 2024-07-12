#version 330 core

out vec4 result;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    result = texture(texture_diffuse1, TexCoords);
}