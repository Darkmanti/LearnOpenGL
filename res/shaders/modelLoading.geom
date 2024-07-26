#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float time;

in VS_OUT
{
    vec2 TexCoords;
} vs_out[];

out GS_OUT
{
    vec2 TexCoords;
} gs_out;

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
} 

void main()
{
    vec3 polygonNormal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, polygonNormal);
    gs_out.TexCoords = vs_out[0].TexCoords;
    EmitVertex();

    gl_Position = explode(gl_in[1].gl_Position, polygonNormal);
    gs_out.TexCoords = vs_out[1].TexCoords;
    EmitVertex();

    gl_Position = explode(gl_in[2].gl_Position, polygonNormal);
    gs_out.TexCoords = vs_out[2].TexCoords;
    EmitVertex();
    EndPrimitive();
}  