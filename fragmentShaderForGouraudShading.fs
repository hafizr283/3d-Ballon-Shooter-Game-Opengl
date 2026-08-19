#version 330 core

in vec3 LightingColor;
in vec2 TexCoords;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform bool isTextured;
uniform int textureMode; 
uniform bool isProcedural;
uniform float texScale;

uniform vec3 viewPos;

// Fog
uniform bool fogEnabled;
uniform vec3 fogColor;
uniform float fogDensity;

void main()
{
    vec3 result = LightingColor;

    if (isProcedural) {
        vec3 V = normalize(viewPos - FragPos);
        // We approximate the N dot V locally or just use a simple radial for Gouraud
        // Actually, Gouraud usually looks less smooth for procedural, but we'll try to match it
        float intensity = max(0.0, dot(vec3(0,1,0), V)); // fallback normal for procedural
        vec3 darkRed = vec3(0.5, 0.0, 0.0);
        vec3 brightRed = vec3(1.0, 0.2, 0.2);
        vec3 colorProgression = mix(darkRed, brightRed, intensity);
        FragColor = vec4(colorProgression * result, 1.0);
    } 
    else if (isTextured) {
        vec2 scaledTexCoords = TexCoords * texScale;
        vec4 texColor = texture(diffuseMap, scaledTexCoords);
        if (textureMode == 1) { 
            FragColor = texColor; 
        } else if (textureMode == 2) { 
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
