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

#define BASE 1
#define DYNAMIC 2
typedef struct {
    glm::vec3 position;
    glm::vec3 color;
} Light;

struct Model {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;
    
    Shader shader;
    std::vector<Mesh> meshes;
    glm::mat4 baseModel;
    glm::mat4 dynamicModel;
    
    bool valid = false;
    
    Model(std::string model_file, const char* vertexPath, const char* fragmentPath);
    void set_identity(int model_id);
    void setTransform(glm::vec3 scale, glm::vec3 rotationAxis, float angleDeg, glm::vec3 translation, int model_id);
    void translate(glm::vec3 translate_vector, int model_id);
    void scale(glm::vec3 scale_vector, int model_id);
    void rotate(GLfloat angle, glm::vec3 rotate_vector, int model_id);

    void draw(
        glm::mat4 &view,
        glm::mat4 &projection,
        glm::vec3 lightPosition,
        glm::vec3 lightColor,
        glm::vec3 viewPosition
    );

    void destroy();
};

Model::Model(std::string model_file, const char* vertexPath, const char* fragmentPath): shader(vertexPath, fragmentPath) {
    meshes = generate_mesh_from_file(model_file);

    if(shader.initialized) {
        baseModel = glm::mat4(1.0f);
        dynamicModel = glm::mat4(1.0f);
        valid = true;
    }
};

void Model::set_identity(int model_id = BASE) {
    if (model_id == BASE)
        baseModel = glm::mat4(1.0f);
    else
        dynamicModel = glm::mat4(1.0f);
}

void Model::setTransform(glm::vec3 scale, glm::vec3 rotationAxis, float angleDeg, glm::vec3 translation, int model_id = BASE) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translation);
    model = glm::rotate(model, glm::radians(angleDeg), rotationAxis);
    model = glm::scale(model, scale);

    if (model_id == BASE)
        baseModel = model;
    else
        dynamicModel = model;
}


void Model::translate(glm::vec3 translate_vector, int model_id = BASE) {
    if (model_id == BASE)
        baseModel = glm::translate(baseModel, translate_vector);
    else
        dynamicModel = glm::translate(dynamicModel, translate_vector);
}

void Model::scale(glm::vec3 scale_vector, int model_id = BASE) {
    if (model_id == BASE)
        baseModel = glm::scale(baseModel, scale_vector);
    else
        dynamicModel = glm::scale(dynamicModel, scale_vector);
}

void Model::rotate(GLfloat angle, glm::vec3 rotate_vector, int model_id = BASE) {
    if (model_id == BASE)
        baseModel = glm::rotate(baseModel, glm::radians(angle), rotate_vector);
    else
        dynamicModel = glm::rotate(dynamicModel, glm::radians(angle), rotate_vector);
}

void Model::draw(
    glm::mat4 &view,
    glm::mat4 &projection,
    glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 50.0f),
    glm::vec3 lightColor = glm::vec3(1.0f),
    glm::vec3 viewPosition = glm::vec3(0.0f, 0.0f, 50.0f)
) {
    shader.use();
    
    glm::mat4 model = baseModel * dynamicModel;
    shader.setMat4("model", glm::value_ptr(model));
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    shader.setVec3("lightPosition", lightPosition);
    shader.setVec3("lightColor", lightColor);

    shader.setVec3("viewPos", viewPosition);
    
    for (const auto& mesh: meshes) {
        mesh.draw(shader);
    }
}

void Model::destroy() {
    for (const auto& mesh: meshes)
        mesh.destroy_mesh();
    glDeleteProgram(shader.ID);
}

#endif