#include "model.hpp"
#include <stack>
#define RESTITUTION 0.6f
#define FRICTION 0.8f

bool checkAABBCollision(const AABB &a, const AABB &b)
{
    return (a.min_corner.x <= b.max_corner.x && a.max_corner.x >= b.min_corner.x) &&
           (a.min_corner.y <= b.max_corner.y && a.max_corner.y >= b.min_corner.y) &&
           (a.min_corner.z <= b.max_corner.z && a.max_corner.z >= b.min_corner.z);
}

bool recursiveAABBTreeCollision(
    AABBNode* aRoot, AABBNode* bNode,
    const glm::mat4& aTransform,
    const glm::mat4& bTransform,
    glm::vec3& collisionNormal,
    float& penetrationDepth
) {
    if (!aRoot || !bNode)
        return false;

    std::stack<std::pair<AABBNode*, AABBNode*>> stack;
    stack.push({aRoot, bNode});

    while (!stack.empty()) {
        auto [aNode, bNode] = stack.top();
        stack.pop();

        // Obter AABB transformadas inline
        AABB aAABB = aNode->getTransformed(aTransform);
        AABB bAABB = bNode->getTransformed(bTransform);

        if (!checkAABBCollision(aAABB, bAABB))
            continue;

        if (aNode->isLeaf() && bNode->isLeaf()) {
            // Calcular penetração
            float xPen = std::min(aAABB.max_corner.x, bAABB.max_corner.x) - std::max(aAABB.min_corner.x, bAABB.min_corner.x);
            float yPen = std::min(aAABB.max_corner.y, bAABB.max_corner.y) - std::max(aAABB.min_corner.y, bAABB.min_corner.y);
            float zPen = std::min(aAABB.max_corner.z, bAABB.max_corner.z) - std::max(aAABB.min_corner.z, bAABB.min_corner.z);

            float minPen = std::min({xPen, yPen, zPen});
            if (minPen <= 0.0f) continue;

            penetrationDepth = minPen;

            if (minPen == xPen) collisionNormal = glm::vec3(1, 0, 0);
            else if (minPen == yPen) collisionNormal = glm::vec3(0, 1, 0);
            else collisionNormal = glm::vec3(0, 0, 1);

            return true;
        }

        // Expandir nós filhos
        if (!aNode->isLeaf() && !bNode->isLeaf()) {
            stack.push({aNode->left, bNode->left});
            stack.push({aNode->left, bNode->right});
            stack.push({aNode->right, bNode->left});
            stack.push({aNode->right, bNode->right});
        }
        else if (!aNode->isLeaf()) {
            stack.push({aNode->left, bNode});
            stack.push({aNode->right, bNode});
        }
        else if (!bNode->isLeaf()) {
            stack.push({aNode, bNode->left});
            stack.push({aNode, bNode->right});
        }
    }

    return false;
}

void checkCollisionWithSceneBounds(Model &model, const AABB &sceneAABB)
{
    AABB modelAABB = model.getGlobalAABB();

    glm::vec3 correction(0.0f);

    // X axis
    if (modelAABB.min_corner.x < sceneAABB.min_corner.x && model.object.velocity.x < 0.0f)
    {
        correction.x = sceneAABB.min_corner.x - modelAABB.min_corner.x;
        model.object.velocity.x *= -RESTITUTION;
    }
    else if (modelAABB.max_corner.x > sceneAABB.max_corner.x && model.object.velocity.x > 0.0f)
    {
        correction.x = sceneAABB.max_corner.x - modelAABB.max_corner.x;
        model.object.velocity.x *= -RESTITUTION;
    }

    // Y axis
    if (modelAABB.min_corner.y < sceneAABB.min_corner.y && model.object.velocity.y < 0.0f)
    {
        correction.y = sceneAABB.min_corner.y - modelAABB.min_corner.y;
        model.object.velocity.y *= -RESTITUTION;

        model.object.velocity.x *= FRICTION;
        model.object.velocity.z *= FRICTION;

        if (glm::abs(model.object.velocity.y) < 0.1f)
        {
            model.object.velocity.y = 0.0f;
        }
    }
    else if (modelAABB.max_corner.y > sceneAABB.max_corner.y && model.object.velocity.y > 0.0f)
    {
        correction.y = sceneAABB.max_corner.y - modelAABB.max_corner.y;
        model.object.velocity.y *= -RESTITUTION;
    }

    // Z axis
    if (modelAABB.min_corner.z < sceneAABB.min_corner.z && model.object.velocity.z < 0.0f)
    {
        correction.z = sceneAABB.min_corner.z - modelAABB.min_corner.z;
        model.object.velocity.z *= -1.0f;
    }
    else if (modelAABB.max_corner.z > sceneAABB.max_corner.z && model.object.velocity.z > 0.0f)
    {
        correction.z = sceneAABB.max_corner.z - modelAABB.max_corner.z;
        model.object.velocity.z *= -1.0f;
    }

    // Corrigir posição se necessário
    if (glm::length(correction) > 0.0f)
    {
        model.translate(correction);
    }
}

bool modelsCollided(Model &a, Model &b) {
    glm::mat4 aModelMatrix = a.effect * a.model;
    glm::mat4 bModelMatrix = b.effect * b.model;

    bool collided = false;
    float penetration = 0.0f;

    for (const auto& meshA : a.meshes) {
        for (const auto& meshB : b.meshes) {
            glm::vec3 localNormal;
            float localPenetration;

            if (recursiveAABBTreeCollision(meshA.boundingTree, meshB.boundingTree, aModelMatrix, bModelMatrix, localNormal, localPenetration)) {
                if (!collided || localPenetration > penetration) {
                    penetration = localPenetration;
                    collided = true;
                }
            }
        }
    }

    return collided;
}

void handleModelCollisionPrecise(Model &a, Model &b) {
    glm::mat4 aModelMatrix = a.effect * a.model;
    glm::mat4 bModelMatrix = b.effect * b.model;

    bool collided = false;
    glm::vec3 collisionNormal(0.0f);
    float penetration = 0.0f;

    for (const auto& meshA : a.meshes) {
        for (const auto& meshB : b.meshes) {
            glm::vec3 localNormal;
            float localPenetration;

            if (recursiveAABBTreeCollision(meshA.boundingTree, meshB.boundingTree, aModelMatrix, bModelMatrix, localNormal, localPenetration)) {
                if (!collided || localPenetration > penetration) {
                    collisionNormal = localNormal;
                    penetration = localPenetration;
                    collided = true;
                }
            }
        }
    }

    if (!collided) return;

    // Corrige direção do vetor de colisão
    glm::vec3 delta = a.getPosition() - b.getPosition();
    if (glm::dot(delta, collisionNormal) < 0.0f)
        collisionNormal = -collisionNormal;

    glm::vec3 correction = collisionNormal * penetration;

    // Corrige posição com base na massa
    float totalMass = a.object.mass + b.object.mass;
    float aWeight = b.object.mass / totalMass;
    float bWeight = a.object.mass / totalMass;

    a.translate(correction * aWeight);
    b.translate(-correction * bWeight);

    // Aplica impulso
    glm::vec3 relativeVelocity = a.object.velocity - b.object.velocity;
    float separatingVelocity = glm::dot(relativeVelocity, collisionNormal);

    if (separatingVelocity < 0.0f) {
        float impulse = -(1.0f + RESTITUTION) * separatingVelocity;
        impulse /= (1.0f / a.object.mass + 1.0f / b.object.mass);

        glm::vec3 impulseVec = collisionNormal * impulse;

        a.object.velocity += impulseVec / a.object.mass;
        b.object.velocity -= impulseVec / b.object.mass;
    }
}
