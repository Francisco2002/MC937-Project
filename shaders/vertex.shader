#version 330 core

layout (location = 0) in vec3 aPos;  // Posição do vértice
layout (location = 1) in vec2 aText;  // Textura do vértice
layout (location = 2) in vec3 aNormal;  // Normal do vértice

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out vec3 FragNormal;
out vec3 FragPosition;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);  // Transformação final
    TexCoord = aText;
    FragNormal = aNormal;
    FragPosition = vec3(model * vec4(aPos, 1.0));
}
