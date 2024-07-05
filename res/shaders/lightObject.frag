#version 330 core

out vec4 result;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Flashlight
{
    vec3 position;
    vec3 direction;
    float innerCutOff;
    float outerCutOff;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform Flashlight flashlight;

vec3 CalculateFlashlight()
{
    vec3 lightDir = normalize(flashlight.position - FragPos);
    float theta = dot(lightDir, normalize(-flashlight.direction));

    vec3 flashlightResult = vec3(0.0, 0.0, 0.0);

    if(theta > flashlight.outerCutOff)
    {
        // Diffuse 
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
        
        // Specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;

        // Soft edges
        float epsilon = (flashlight.innerCutOff - flashlight.outerCutOff);
        float intensity = clamp((theta - flashlight.outerCutOff) / epsilon, 0.0, 1.0);

        diffuse  *= intensity;
        specular *= intensity;
        
        // Attenuation
        float distanceToPoint = length(flashlight.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distanceToPoint + light.quadratic * (distanceToPoint * distanceToPoint));

        diffuse *= attenuation;
        specular *= attenuation;

        // Result
        flashlightResult = diffuse + specular;
    }

    return flashlightResult;
}

void main()
{
    // Attenuation
    float distanceToPoint = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distanceToPoint + light.quadratic * (distanceToPoint * distanceToPoint));

    // Ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoord).rgb;

    // Use attenuation
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Flashlight
    vec3 flashlightResult = CalculateFlashlight();

    // Result
    result = vec4(ambient + diffuse + specular + flashlightResult, 1.0);
}