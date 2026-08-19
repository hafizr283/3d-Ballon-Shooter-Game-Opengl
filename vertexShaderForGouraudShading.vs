#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 LightingColor;
out vec2 TexCoords;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float k_c;
    float k_l;
    float k_q;
    int lightNumber;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float k_c;
    float k_l;
    float k_q;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform PointLight pointLights[3];
uniform DirectionalLight dirLight;
uniform SpotLight spotLight;
uniform Material material;

vec3 CalcPointLight(Material material, PointLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcDirLight(Material material, DirectionalLight light, vec3 N, vec3 V);
vec3 CalcSpotLight(Material material, SpotLight light, vec3 N, vec3 Pos, vec3 V);

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    vec3 Pos = vec3(model * vec4(aPos, 1.0));
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    
    TexCoords = aTexCoords;
    FragPos = Pos;
    
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - Pos);

    vec3 result = CalcDirLight(material, dirLight, N, V);
    for(int i = 0; i < 2; i++)
        result += CalcPointLight(material, pointLights[i], N, Pos, V);
    result += CalcSpotLight(material, spotLight, N, Pos, V);
   
    LightingColor = result;
}

vec3 CalcPointLight(Material material, PointLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    vec3 R = reflect(-L, N);
    
    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);
    
    vec3 ambient = material.ambient * light.ambient;
    vec3 diffuse = material.diffuse * max(0.0, dot(N, L)) * light.diffuse;
    vec3 specular = material.specular * pow(max(dot(V, R), 0.0), material.shininess) * light.specular;
    
    return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcDirLight(Material material, DirectionalLight light, vec3 N, vec3 V)
{
    vec3 L = normalize(-light.direction);
    vec3 R = reflect(-L, N);
    
    vec3 ambient = material.ambient * light.ambient;
    vec3 diffuse = material.diffuse * max(0.0, dot(N, L)) * light.diffuse;
    vec3 specular = material.specular * pow(max(dot(V, R), 0.0), material.shininess) * light.specular;
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Material material, SpotLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    vec3 R = reflect(-L, N);
    
    float d = length(light.position - Pos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);
    
    float theta = dot(L, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    vec3 ambient = material.ambient * light.ambient * attenuation; 
    vec3 diffuse = material.diffuse * max(0.0, dot(N, L)) * light.diffuse * attenuation * intensity;
    vec3 specular = material.specular * pow(max(dot(V, R), 0.0), material.shininess) * light.specular * attenuation * intensity;
    
    return (ambient + diffuse + specular);
}
