#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

out vec3 VertexPosition;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 mvpMatrix;
uniform mat4 model;

void main()
{
    VertexPosition = vec3(model * vec4(inPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * inNormal;
    TexCoord = inTexCoord;

    gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}