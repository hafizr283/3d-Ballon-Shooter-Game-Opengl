#version 330 core
layout (location = 0) in vec3 aPos;

out vec4 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 darkBlue = vec3(0.0, 0.0, 0.5);
    vec3 lightBlue = vec3(0.5, 0.8, 1.0);
    // R3: Color computed on the vertex
    float t = aPos.y + 0.5; // Map from [-0.5, 0.5] to [0.0, 1.0]
    vertexColor = vec4(mix(lightBlue, darkBlue, t), 1.0);
}
