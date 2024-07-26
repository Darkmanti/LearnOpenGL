#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in VertexData
{
    vec3 color;
} inVertexData[];

out VertexData
{
    vec3 color;
} outVertexData;

void build_house(vec4 position)
{
    outVertexData.color = inVertexData[0].color;

    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);    // 1:bottom-left
    EmitVertex();
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0);    // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0);    // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0);    // 4:top-right
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0);    // 5:top
    outVertexData.color = vec3(1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    build_house(gl_in[0].gl_Position);
}