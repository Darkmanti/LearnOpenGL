#version 330 core

out vec4 result;

in VertexData
{
    vec3 VertexPosition;
    vec3 Normal;
    vec2 TexCoord;
} inVertexData;

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
    vec3 normal = normalize(inVertexData.Normal);

    vec3 reflection = CalculateEnvironmentReflection(viewPos, inVertexData.VertexPosition, normal);
    vec3 refraction = CalculateEnvironmentRefraction(viewPos, inVertexData.VertexPosition, normal);

    // Result.
    result = vec4(refraction, 1.0);
}