#version 330 core

in vec3 fragNormal;  // Normal do fragmento
in vec3 fragPos;  // Posição do fragmento

out vec4 fragColor;

uniform vec3 lightPos;  // Posição da luz
uniform vec3 viewPos;   // Posição da câmera

void main()
{
    // Luz direcional simples (iluminação difusa)
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, lightDir), 0.0);  // Cálculo da luz difusa

    // Cor final (cor baseada na luz difusa)
    vec3 color = vec3(0.5f, 0.5f, 0.5f);  // Cor do objeto (exemplo)
    fragColor = vec4(diff * color, 1.0);  // Aplicando a iluminação
}
