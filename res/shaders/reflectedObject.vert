#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

out VertexData
{
    vec3 VertexPosition;
    vec3 Normal;
    vec2 TexCoord;
} outVertexData;

uniform mat4 mvpMatrix;
uniform mat4 model;

void main()
{
    outVertexData.VertexPosition = vec3(model * vec4(inPosition, 1.0));
    outVertexData.Normal = mat3(transpose(inverse(model))) * inNormal;
    outVertexData.TexCoord = inTexCoord;

    gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}