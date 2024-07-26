#version 330 core

out vec4 result;

in VS_OUT
{
    vec2 TexCoords;
} vs_out;

uniform sampler2D texture_diffuse1;

void main()
{
    result = texture(texture_diffuse1, vs_out.TexCoords);
}