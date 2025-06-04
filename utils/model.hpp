#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include "shaders.hpp"
#include "mesh.hpp"
#include "read_obj_file.hpp"

struct Model {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;
    
    Shader shader;
    Mesh mesh;
    glm::mat4 model;

    bool valid = false;
    
    Model(std::string model_file, const char* vertexPath, const char* fragmentPath);
    void translate(glm::vec3 translate_vector);
    void scale(glm::vec3 scale_vector);
    void rotate(GLfloat angle, glm::vec3 rotate_vector);

    void draw(
        glm::mat4 &view,
        glm::mat4 &projection,
        glm::vec3 lightPosition,
        glm::vec3 viewPosition
    );

    void destroy();
};

Model::Model(std::string model_file, const char* vertexPath, const char* fragmentPath): shader(vertexPath, fragmentPath), mesh(generate_mesh_from_file(model_file)) {
    if(shader.initialized) {
        model = glm::mat4(1.0f);
        valid = true;
    }
};

void Model::translate(glm::vec3 translate_vector) {
    model = glm::translate(model, translate_vector);
}

void Model::scale(glm::vec3 scale_vector) {
    model = glm::scale(model, scale_vector);
}

void Model::rotate(GLfloat angle, glm::vec3 rotate_vector) {
    model = glm::rotate(model, glm::radians(angle), rotate_vector);
}

void Model::draw(
    glm::mat4 &view,
    glm::mat4 &projection,
    glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 50.0f),
    glm::vec3 viewPosition = glm::vec3(0.0f, 0.0f, 50.0f)
) {
    shader.use();
    shader.setMat4("model", glm::value_ptr(model));
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    shader.setVec3("lightPos", lightPosition);
    shader.setVec3("viewPos", viewPosition);

    mesh.draw(shader);
}

void Model::destroy() {
    mesh.destroy_mesh();
    glDeleteProgram(shader.ID);
}

#endif