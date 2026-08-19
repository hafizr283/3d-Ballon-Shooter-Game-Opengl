#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

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
    float k_c;  // attenuation factors
    float k_l;  // attenuation factors
    float k_q;  // attenuation factors
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

uniform vec3 viewPos;
uniform PointLight pointLights[3];
uniform DirectionalLight dirLight;
uniform SpotLight spotLight;
uniform Material material;

uniform sampler2D diffuseMap;
uniform bool isTextured;
uniform int textureMode; // 1 = GL_REPLACE (Texture only), 2 = GL_MODULATE (Texture * Base Color)
uniform vec4 vertexColor; // For R3 procedural/vertex color overriding (if we choose to use it here)
uniform bool isProcedural; // For R4 procedural red radial gradient
uniform float texScale; // For dynamically scaling/repeating textures

// Fog
uniform bool fogEnabled;
uniform vec3 fogColor;
uniform float fogDensity;

// function prototypes
vec3 CalcPointLight(Material material, PointLight light, vec3 N, vec3 Pos, vec3 V);
vec3 CalcDirLight(Material material, DirectionalLight light, vec3 N, vec3 V);
vec3 CalcSpotLight(Material material, SpotLight light, vec3 N, vec3 FragPos, vec3 V);

void main()
{
     // properties
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result;
    
    // directional light
    result = CalcDirLight(material, dirLight, N, V);

    // point lights
    for(int i = 0; i < 2; i++)
        result += CalcPointLight(material, pointLights[i], N, FragPos, V);
        
    // spot light
    result += CalcSpotLight(material, spotLight, N, FragPos, V);

    if (isProcedural) {
        // Procedural radial gradient (R4 parameter simulation)
        // Let's create a red gradient from center to edge based on FragPos mapping or normalized normal
        float intensity = max(0.0, dot(N, V)); // center usually faces view
        vec3 darkRed = vec3(0.5, 0.0, 0.0);
        vec3 brightRed = vec3(1.0, 0.2, 0.2);
        vec3 colorProgression = mix(darkRed, brightRed, intensity);
        FragColor = vec4(colorProgression * result, 1.0);
    } 
    else if (isTextured) {
        vec2 scaledTexCoords = TexCoords * texScale;
        vec4 texColor = texture(diffuseMap, scaledTexCoords);
        if (textureMode == 1) { // GL_REPLACE (No lighting, just texture. Grass, Sky, Asphalt)
            FragColor = texColor; 
        } else if (textureMode == 2) { // GL_MODULATE (Lighting * Texture. Brick, Bark)
            FragColor = vec4(result * texColor.rgb, texColor.a);
        } else {
            FragColor = vec4(result, 1.0);
        }
    } else {
        FragColor = vec4(result, 1.0);
    }

    // Apply fog
    if (fogEnabled) {
        float dist = length(viewPos - FragPos);
        float fogFactor = exp(-fogDensity * dist);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        FragColor = vec4(mix(fogColor, FragColor.rgb, fogFactor), FragColor.a);
    }
}

// calculates the color when using a point light.
vec3 CalcPointLight(Material material, PointLight light, vec3 N, vec3 Pos, vec3 V)
{
    vec3 L = normalize(light.position - Pos);
    vec3 R = reflect(-L, N);
    
    vec3 K_A = material.ambient;
    vec3 K_D = material.diffuse;
    vec3 K_S = material.specular;
    
    // attenuation
    float d = length(light.position - Pos);
    float kc = light.k_c;
    float kl = light.k_l;
    float kq = light.k_q;


    float attenuation = 1/(kc+kl*d+kq*d*d);
    
    vec3 ambient = K_A * light.ambient;
    vec3 diffuse = K_D * max(0,dot(N, L)) * light.diffuse;
    vec3 specular = K_S * pow(max(dot(V, R), 0.0), material.shininess) * light.specular;

    

    
   // ambient *= attenuation;
   // diffuse *= attenuation;
   // specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

vec3 CalcDirLight(Material material, DirectionalLight light, vec3 N, vec3 V)
{
    vec3 L = normalize(-light.direction);
    vec3 R = reflect(-L, N);
    
    vec3 K_A = material.ambient;
    vec3 K_D = material.diffuse;
    vec3 K_S = material.specular;
    
    vec3 ambient = K_A * light.ambient;
    vec3 diffuse = K_D * max(0,dot(N, L)) * light.diffuse;
    vec3 specular = K_S * pow(max(dot(V, R), 0.0), material.shininess) * light.specular;
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Material material, SpotLight light, vec3 N, vec3 FragPos, vec3 V)
{
    vec3 L = normalize(light.position - FragPos);
    vec3 R = reflect(-L, N);
    
    // attenuation
    float d = length(light.position - FragPos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * d * d);
    
    // spotlight intensity
    float theta = dot(L, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    vec3 K_A = material.ambient;
    vec3 K_D = material.diffuse;
    vec3 K_S = material.specular;
    
    vec3 ambient = K_A * light.ambient * attenuation; 
    
    vec3 diffuse = K_D * max(0,dot(N, L)) * light.diffuse * attenuation * intensity;
    vec3 specular = K_S * pow(max(dot(V, R), 0.0), material.shininess) * light.specular * attenuation * intensity;
    
    return (ambient + diffuse + specular);
}

