#version 330 core

layout (location = 0) in vec3 inPosition;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = inPosition;
    vec4 position = projection * view * vec4(inPosition, 1.0);

    // Set z conponent to w, then after perspective division get depth == 1.0f.
    gl_Position = position.xyww;
}