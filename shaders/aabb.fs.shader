#version 330 core

out vec4 FragColor;

uniform vec3 color;  // Cor sólida para a AABB

void main() {
    FragColor = vec4(color, 1.0);
}
