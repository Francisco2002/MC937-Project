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
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 displacedPosition = glm::vec3(0.0f);// posição suavizada com inércia
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 size = glm::vec3(0.0f);
    GLfloat mass = 0; 
    
    glm::vec3 angularVelocity = glm::vec3(0.0f);  // Velocidade angular (torque)
    GLfloat angularDamping = 0.99f;  // Damping para as rotações (ajuste conforme necessário)

    void incrementVelocity(glm::vec3 v);
};

void Object::incrementVelocity(glm::vec3 v = glm::vec3(0.01f)) {
    velocity += v;
}

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
    Object object;
    Shader shader;
    Shader aabbShader;
    std::vector<Mesh> meshes;
    AABB modelAABB;

    glm::mat4 model;

    // non-cumulative effect matrix
    glm::mat4 effect;
    
    // cumulative transforms
    Transforms transforms;
    
    bool valid = false;
    
    Model(std::string model_file, const char* vertexPath, const char* fragmentPath);

    void setModelMass(GLfloat mass);

    void translate(glm::vec3 translate_vector);
    void scale(glm::vec3 scale_vector);
    void rotate(GLfloat angle, glm::vec3 rotate_vector);

    void applyEffect(TransformType type, glm::vec3 t, GLfloat a);
    void clearEffect();
    void updateModelMatrix();

    glm::vec3 getPosition() const;

    void draw(
        glm::mat4 &view,
        glm::mat4 &projection,
        glm::vec3 lightPosition,
        glm::vec3 lightColor,
        glm::vec3 viewPosition,
        bool showAABB
    );

    void setInitialGlobalAABB();
    AABB getGlobalAABB();
    void destroy();
};

void Model::setModelMass(GLfloat mass) {
    object.mass = mass;
}

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

glm::vec3 Model::getPosition() const {
    return glm::vec3(model[3]); // Pega a coluna da translação
}

void Model::setInitialGlobalAABB() {
    if (meshes.empty()) {
        modelAABB.min_corner = glm::vec3(0.0f);
        modelAABB.max_corner = glm::vec3(0.0f);
    }

    glm::vec3 globalMin(FLT_MAX);
    glm::vec3 globalMax(-FLT_MAX);

    auto transformAABBNode = [&](const AABBNode* node, auto&& self_ref) -> void {
        if (!node) return;

        const glm::vec3& min = node->box.min_corner;
        const glm::vec3& max = node->box.max_corner;

        glm::vec3 corners[8] = {
            {min.x, min.y, min.z}, {max.x, min.y, min.z},
            {max.x, max.y, min.z}, {min.x, max.y, min.z},
            {min.x, min.y, max.z}, {max.x, min.y, max.z},
            {max.x, max.y, max.z}, {min.x, max.y, max.z}
        };

        for (const glm::vec3& corner : corners) {
            globalMin = glm::min(globalMin, corner);
            globalMax = glm::max(globalMax, corner);
        }

        self_ref(node->left, self_ref);
        self_ref(node->right, self_ref);
    };

    for (const auto& mesh : meshes) {
        transformAABBNode(mesh.boundingTree, transformAABBNode);
    }

    modelAABB.min_corner = globalMin;
    modelAABB.max_corner = globalMax;
}

AABB Model::getGlobalAABB() {
    glm::mat4 transform = effect * model;
     
    glm::vec3 min_corner = glm::vec3(transform * glm::vec4(modelAABB.min_corner, 1.0f));
    glm::vec3 max_corner = glm::vec3(transform * glm::vec4(modelAABB.max_corner, 1.0f));

    return AABB{min_corner, max_corner};
}

Model::Model(std::string model_file, const char* vertexPath, const char* fragmentPath): shader(vertexPath, fragmentPath), aabbShader("shaders/aabb.vs.shader", "shaders/aabb.fs.shader") {
    meshes = generate_mesh_from_file(model_file);

    if(shader.initialized) {
        model = glm::mat4(1.0f);
        effect = glm::mat4(1.0f);
        valid = true;

        setInitialGlobalAABB();
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
    
    for (const auto& mesh: meshes) {
        mesh.draw(shader);
        if(showAABB) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            mesh.drawBoundingTree(shader, model, view, projection);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void Model::destroy() {
    for (auto& mesh: meshes)
        mesh.destroy_mesh();
    glDeleteProgram(shader.ID);
    glDeleteProgram(aabbShader.ID);
}

#endif