#version 330 core

out vec4 result;

in VertexData
{
    vec3 color;
} inVertexData;

void main()
{
    result = vec4(inVertexData.color, 1.0);
}