#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp"

#if __has_include(<filesystem>)
    #include <filesystem>
    namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #error "Nenhum suporte a filesystem encontrado!"
#endif

struct Vertex {
    glm::vec3 position;
    glm::vec2 textureCoord;
    glm::vec3 normal;
};

struct Material {
    glm::vec3 ambient = glm::vec3(0.0f);
    glm::vec3 diffuse = glm::vec3(0.0f);
    glm::vec3 specular = glm::vec3(0.0f);
    std::string diffuseTexturePath = "";
    std::string specularTexturePath = "";
    std::string bumpTexturePath = "";
};

struct AABB {
    glm::vec3 max_corner;
    glm::vec3 min_corner;
};

// represent any drawable object
struct Mesh {
    std::string name;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<GLuint> textures;
    Material material;

    GLuint VAO, VBO, EBO;

    AABB boundingBox;

    Mesh(std::string n, const std::vector<Vertex> &v, const std::vector<GLuint> &i, const Material& m, const std::string& baseDir);
    Mesh(const std::vector<Vertex> &v, const std::vector<GLuint> &i, const Material& m, const std::string& baseDir);

    void draw(const Shader &s) const;
    void setup_mesh();
    void load_texture(const char* path);
    void destroy_mesh() const;

    void computeAABB();          // <-- Adicionado
    void drawAABB(const Shader& s, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const;
};

void Mesh::computeAABB() {
    if (vertices.empty()) return;

    glm::vec3 min = vertices[0].position;
    glm::vec3 max = vertices[0].position;

    for (const auto& vertex : vertices) {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    boundingBox.min_corner = min;
    boundingBox.max_corner = max;
}

void Mesh::drawAABB(const Shader& s, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const {
    // Canto mínimo e máximo da AABB no espaço do objeto
    const glm::vec3& min = boundingBox.min_corner;
    const glm::vec3& max = boundingBox.max_corner;

    // 8 vértices do cubo AABB
    glm::vec3 corners[8] = {
        {min.x, min.y, min.z}, // 0
        {max.x, min.y, min.z}, // 1
        {max.x, max.y, min.z}, // 2
        {min.x, max.y, min.z}, // 3
        {min.x, min.y, max.z}, // 4
        {max.x, min.y, max.z}, // 5
        {max.x, max.y, max.z}, // 6
        {min.x, max.y, max.z}  // 7
    };

    // 12 arestas (pares de índices)
    GLuint indices[] = {
        0,1, 1,2, 2,3, 3,0, // base inferior
        4,5, 5,6, 6,7, 7,4, // base superior
        0,4, 1,5, 2,6, 3,7  // colunas verticais
    };

    // Criar buffers temporários
    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);

    // Posição (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Configurar shader
    s.use();
    s.setMat4("model", glm::value_ptr(model));  // você precisa passar uma matriz modelo
    s.setMat4("view", glm::value_ptr(view));  // você precisa passar uma matriz modelo
    s.setMat4("projection", glm::value_ptr(projection));  // você precisa passar uma matriz modelo

    s.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f)); // vermelho (ou qualquer cor)

    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // Limpar buffers temporários
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

Mesh::Mesh(std::string n, const std::vector<Vertex> &v, const std::vector<GLuint> &i, const Material& m, const std::string& baseDir): Mesh(v, i, m, baseDir) {
    name = n;
}

Mesh::Mesh(const std::vector<Vertex> &v, const std::vector<GLuint> &i, const Material& m, const std::string& baseDir): vertices(v), indices(i), material(m) {
    if (!m.diffuseTexturePath.empty()) {
        fs::path base(baseDir);
        fs::path relative(material.diffuseTexturePath);

        fs::path fullPath = fs::weakly_canonical(base / relative);
        load_texture(fullPath.string().c_str());
    }

    if (!m.specularTexturePath.empty()) {
        fs::path base(baseDir);
        fs::path relative(m.specularTexturePath);

        fs::path fullPath = fs::weakly_canonical(base / relative);
        load_texture(fullPath.string().c_str());
    }

    if (!m.bumpTexturePath.empty()) {
        fs::path base(baseDir);
        fs::path relative(m.bumpTexturePath);

        fs::path fullPath = fs::weakly_canonical(base / relative);
        load_texture(fullPath.string().c_str());
    }
    
    setup_mesh();
    computeAABB();
}

void Mesh::draw(const Shader &s) const {
    s.setVec3("material.ambient", material.ambient);
    s.setVec3("material.diffuse", material.diffuse);
    s.setVec3("material.specular", material.specular);

    s.setInt("material.hasDiffuseTexture", !material.diffuseTexturePath.empty());
    s.setInt("material.hasSpecularTexture", !material.specularTexturePath.empty());
    s.setInt("material.hasBumpTexture", !material.bumpTexturePath.empty());

    int texIndex = 0;

    if (!material.diffuseTexturePath.empty()) {
        glActiveTexture(GL_TEXTURE0 + texIndex);
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        s.setInt("material.diffuseTexture", texIndex);
        texIndex++;
    }

    if (!material.specularTexturePath.empty()) {
        glActiveTexture(GL_TEXTURE0 + texIndex);
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        s.setInt("material.specularTexture", texIndex);
        texIndex++;
    }

    if (!material.bumpTexturePath.empty()) {
        glActiveTexture(GL_TEXTURE0 + texIndex);
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        s.setInt("material.bumpTexture", texIndex);
        texIndex++;
    }
    
    /* for(int i = 0; i < textures.size(); i++) {
        GLuint texture = textures[i];

        // Ativar a unidade de textura
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Configurar o uniform no shader (uma vez)
        s.setInt("texture" + std::to_string(i + 1), i);
    } */

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setup_mesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // textures
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, textureCoord)));
    glEnableVertexAttribArray(1);

    // normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::load_texture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        
        if (nrChannels == 1) {
            format = GL_RED;
        } else if (nrChannels == 3) {
            format = GL_RGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        } else {
            std::cerr << "Erro: número de canais inválido: " << nrChannels << std::endl;
            stbi_image_free(data);
            return;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parâmetros de textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        textures.push_back(textureID);

        stbi_image_free(data);
    } else {
        std::cerr << "Erro ao carregar textura: " << path << std::endl;
        stbi_image_free(data);
    }
}

void Mesh::destroy_mesh() const {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    for (GLuint tex : textures) {
        glDeleteTextures(1, &tex);
    }
}

#endif