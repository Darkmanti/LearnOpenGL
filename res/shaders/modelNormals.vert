#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

out VS_OUT
{
    vec3 Normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = view * model * vec4(inPosition, 1.0);

    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.Normal = normalize(vec3(vec4(normalMatrix * inNormal, 0.0)));
}