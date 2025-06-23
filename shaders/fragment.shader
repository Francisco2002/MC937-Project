#version 330 core

in vec2 TexCoord;
in vec3 FragNormal;
in vec3 FragPosition;

uniform vec3 viewPosition;
uniform vec3 lightPosition;
uniform vec3 lightColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    sampler2D diffuseTexture;
    sampler2D specularTexture;
    sampler2D bumpTexture;
    
    int hasDiffuseTexture;
    int hasSpecularTexture;
    int hasBumpTexture;
};

uniform Material material;

uniform sampler2D texture1; // base color

// details
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;

out vec4 fragColor;

void main()
{
    float strenght = 0.8;
    vec3 ambient = material.ambient * lightColor * strenght;
    
    vec3 normal = normalize(FragNormal);

    if (material.hasBumpTexture == 1) {
        // Lê o valor da altura da textura de bump (em escala de cinza)
        float height = texture(material.bumpTexture, TexCoord).r;

        // Calcula gradiente aproximado (usando diferença de altura)
        float strength = 0.05; // Experimente valores entre 0.01 e 0.2
        vec3 bump = vec3(0.0, 0.0, height * strength);

        // Perturba a normal usando o vetor "bump"
        normal = normalize(normal + bump);
    }

    vec3 lightDir = normalize(lightPosition - FragPosition);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * lightColor * diff;

    vec3 viewDir = normalize(viewPosition - FragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    float specularStrength = 0.7;
    if (material.hasSpecularTexture == 1) {
        specularStrength = texture(material.specularTexture, TexCoord).r;
    }

    vec3 specular = material.specular * spec * specularStrength * lightColor;

    vec3 color = ambient + diffuse + specular;

    if (material.hasDiffuseTexture == 1) {
        color *= texture(material.diffuseTexture, TexCoord).rgb;
    }

    fragColor = vec4(color, 1.0);  // Aplicando a iluminação

    // fragColor = vec4(vec3(diff), 1.0); // mostra intensidade da luz como tons de cinza

}
