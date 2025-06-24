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

enum TransformType {
    SCALE,
    TRANSLATE,
    ROTATE
};
struct Light {
    glm::vec3 position;
    glm::vec3 color;
};

struct Object {
    glm::vec3 position;
    glm::vec3 velocity;

    Object(glm::vec3 p);
};

Object::Object(glm::vec3 p):  position(p), velocity(glm::vec3(0.0f)) {}

struct Transforms {
    glm::mat4 translate = glm::mat4(1.0f);
    glm::mat4 rotate = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);

    void addTranslate(const glm::mat4& m) { translate = translate * m; }
    void addRotate(const glm::mat4& m) { rotate = rotate * m; }
    void addScale(const glm::mat4& m) { scale = scale * m; }

    glm::mat4 getModelMatrix() const {
        return translate * rotate * scale;
    }
};

struct Model {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 velocity;
    AABB localAABB;
    
    Shader shader;
    std::vector<Mesh> meshes;
    
    glm::mat4 model;

    // non-cumulative effect matrix
    glm::mat4 effect;
    
    Transforms transforms;
    
    bool valid = false;
    
    Model(std::string model_file, const char* vertexPath, const char* fragmentPath);
    void translate(glm::vec3 translate_vector);
    void scale(glm::vec3 scale_vector);
    void rotate(GLfloat angle, glm::vec3 rotate_vector);

    void applyEffect(TransformType type, glm::vec3 t, GLfloat a);
    void clearEffect();
    void updateModelMatrix();

    void draw(
        glm::mat4 &view,
        glm::mat4 &projection,
        glm::vec3 lightPosition,
        glm::vec3 lightColor,
        glm::vec3 viewPosition,
        bool showAABB
    );

    AABB getGlobalAABB();
    void destroy();
};

void Model::applyEffect(TransformType type, glm::vec3 t, GLfloat a = 0.0f) {
    if (type == SCALE) {
        effect = glm::scale(effect, t);
    } else if (type == TRANSLATE) {
        effect = glm::translate(effect, t);
    } else {
        effect = glm::rotate(effect, glm::radians(a), t);
    }
}

void Model::clearEffect() {
    effect = glm::mat4(1.0f);
}

AABB Model::getGlobalAABB() {
    if (meshes.empty()) return AABB{glm::vec3(0.0f), glm::vec3(0.0f)};

    glm::vec3 globalMin(FLT_MAX);
    glm::vec3 globalMax(-FLT_MAX);

    for (const Mesh& mesh : meshes) {
        AABB localBox = mesh.boundingBox;

        // 8 cantos da AABB local
        glm::vec3 corners[8] = {
            {localBox.min_corner.x, localBox.min_corner.y, localBox.min_corner.z},
            {localBox.max_corner.x, localBox.min_corner.y, localBox.min_corner.z},
            {localBox.max_corner.x, localBox.max_corner.y, localBox.min_corner.z},
            {localBox.min_corner.x, localBox.max_corner.y, localBox.min_corner.z},
            {localBox.min_corner.x, localBox.min_corner.y, localBox.max_corner.z},
            {localBox.max_corner.x, localBox.min_corner.y, localBox.max_corner.z},
            {localBox.max_corner.x, localBox.max_corner.y, localBox.max_corner.z},
            {localBox.min_corner.x, localBox.max_corner.y, localBox.max_corner.z},
        };

        // Transformar cantos pela matriz model
        for (const glm::vec3& corner : corners) {
            glm::vec3 transformed = glm::vec3(effect * model * glm::vec4(corner, 1.0f));

            globalMin = glm::min(globalMin, transformed);
            globalMax = glm::max(globalMax, transformed);
        }
    }

    AABB aabb;
    aabb.min_corner = globalMin;
    aabb.max_corner = globalMax;

    position = (globalMax + globalMin) * 0.5f;
    size = globalMax - globalMin;

    return aabb;
}

Model::Model(std::string model_file, const char* vertexPath, const char* fragmentPath): shader(vertexPath, fragmentPath) {
    meshes = generate_mesh_from_file(model_file);

    if(shader.initialized) {
        model = glm::mat4(1.0f);
        effect = glm::mat4(1.0f);
        valid = true;

        glm::vec3 min(FLT_MAX), max(-FLT_MAX);

        for (const Mesh& mesh : meshes) {
            min = glm::min(min, mesh.boundingBox.min_corner);
            max = glm::max(max, mesh.boundingBox.max_corner);
        }

        localAABB.min_corner = min;
        localAABB.max_corner = max;
    }
};

void Model::updateModelMatrix() {
    model = transforms.getModelMatrix();
}

void Model::translate(glm::vec3 translate_vector) {
    glm::mat4 t = glm::translate(glm::mat4(1.0f), translate_vector);
    transforms.addTranslate(t);
    updateModelMatrix();
}

void Model::scale(glm::vec3 scale_vector) {
    glm::mat4 t = glm::scale(glm::mat4(1.0f), scale_vector);
    transforms.addScale(t);
    updateModelMatrix();
}

void Model::rotate(GLfloat angle, glm::vec3 rotate_vector) {
    glm::mat4 t = glm::rotate(glm::mat4(1.0f), glm::radians(angle), rotate_vector);
    transforms.addRotate(t);
    updateModelMatrix();
}

void Model::draw(
    glm::mat4 &view,
    glm::mat4 &projection,
    glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 50.0f),
    glm::vec3 lightColor = glm::vec3(1.0f),
    glm::vec3 viewPosition = glm::vec3(0.0f, 0.0f, 50.0f),
    bool showAABB = false
) {
    shader.use();

    glm::mat4 modelWithEffect = effect * model;

    shader.setMat4("model", glm::value_ptr(modelWithEffect));
    shader.setMat4("view", glm::value_ptr(view));
    shader.setMat4("projection", glm::value_ptr(projection));

    shader.setVec3("lightPosition", lightPosition);
    shader.setVec3("lightColor", lightColor);

    shader.setVec3("viewPos", viewPosition);

    Shader aabbShader("shaders/aabb.vs.shader", "shaders/aabb.fs.shader");
    
    for (const auto& mesh: meshes) {
        mesh.draw(shader);
        if(showAABB) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            mesh.drawAABB(aabbShader, modelWithEffect, view, projection);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void Model::destroy() {
    for (const auto& mesh: meshes)
        mesh.destroy_mesh();
    glDeleteProgram(shader.ID);
}

#endif