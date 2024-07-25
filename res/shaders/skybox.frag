#version 330 core

out vec4 result;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    result = texture(skybox, TexCoords);
}