#version 330 core

out vec4 result;

in vec3 color;
in vec2 texCoord;

uniform sampler2D albedo;

void main()
{
    result = texture(albedo, texCoord);
}