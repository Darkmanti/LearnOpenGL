#version 330 core

out vec4 result;

in vec3 VertexPosition;
in vec3 Normal;
in vec2 TexCoord;

uniform samplerCube skybox;

uniform vec3 viewPos;

vec3 CalculateEnvironmentReflection(vec3 cameraPosition, vec3 vertexPosition, vec3 normal)
{
    vec3 I = normalize(vertexPosition - cameraPosition);
    vec3 R = reflect(I, normal);

    return texture(skybox, R).rgb;
}

vec3 CalculateEnvironmentRefraction(vec3 cameraPosition, vec3 vertexPosition, vec3 normal)
{
    float ratio = 1.0 / 1.33;
    vec3 I = normalize(vertexPosition - cameraPosition);
    vec3 R = refract(I, normal, ratio);

    return texture(skybox, R).rgb;
}

void main()
{
    vec3 normal = normalize(Normal);

    vec3 reflection = CalculateEnvironmentReflection(viewPos, VertexPosition, normal);
    vec3 refraction = CalculateEnvironmentRefraction(viewPos, VertexPosition, normal);

    // Result.
    result = vec4(refraction, 1.0);
}