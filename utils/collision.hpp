#include "model.hpp"

void printVec(const glm::vec3& v) {
    std::cout << "Vec3: (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

void print_corners(AABB& mbox) {
    std::cout << "==============\n";
    std::cout << "Min: ";
    printVec(mbox.min_corner);

    std::cout << "Max: ";
    printVec(mbox.max_corner);
    std::cout << "==============\n";
}

glm::vec3 translate_to_inside_room(AABB& room, AABB &model) {
    glm::vec3 offset(0.0f);
    const float margin = 0.3f;

    // Eixo X
    if (model.min_corner.x < room.min_corner.x + margin)
        offset.x += (room.min_corner.x + margin) - model.min_corner.x;
    else if (model.max_corner.x > room.max_corner.x - margin)
        offset.x -= model.max_corner.x - (room.max_corner.x - margin);

    // Eixo Y
    if (model.min_corner.y < room.min_corner.y + margin)
        offset.y += (room.min_corner.y + margin) - model.min_corner.y;
    else if (model.max_corner.y > room.max_corner.y - margin)
        offset.y -= model.max_corner.y - (room.max_corner.y - margin);

    // Eixo Z
    if (model.min_corner.z < room.min_corner.z + margin)
        offset.z += (room.min_corner.z + margin) - model.min_corner.z;
    else if (model.max_corner.z > room.max_corner.z - margin)
        offset.z -= model.max_corner.z - (room.max_corner.z - margin);

    return offset;
}

bool is_inside(AABB& m, AABB& s) {
    return (m.min_corner.x >= s.min_corner.x && m.max_corner.x <= s.max_corner.x) &&
           (m.min_corner.y >= s.min_corner.y && m.max_corner.y <= s.max_corner.y) &&
           (m.min_corner.z >= s.min_corner.z && m.max_corner.z <= s.max_corner.z);
}

AABB getTransformedAABB(const Mesh& mesh, const glm::mat4& modelMatrix) {
    const glm::vec3& min = mesh.boundingBox.min_corner;
    const glm::vec3& max = mesh.boundingBox.max_corner;

    // Os 8 cantos da AABB original
    std::vector<glm::vec3> corners = {
        {min.x, min.y, min.z}, {min.x, min.y, max.z}, {min.x, max.y, min.z}, {min.x, max.y, max.z},
        {max.x, min.y, min.z}, {max.x, min.y, max.z}, {max.x, max.y, min.z}, {max.x, max.y, max.z},
    };

    AABB result;
    result.min_corner = glm::vec3(std::numeric_limits<float>::max());
    result.max_corner = glm::vec3(std::numeric_limits<float>::lowest());

    // Transforma e recalcula o AABB no mundo
    for (const auto& corner : corners) {
        glm::vec3 transformed = glm::vec3(modelMatrix * glm::vec4(corner, 1.0f));
        result.min_corner = glm::min(result.min_corner, transformed);
        result.max_corner = glm::max(result.max_corner, transformed);
    }

    return result;
}

bool checkAABBCollision(const AABB& a, const AABB& b) {
    return (a.min_corner.x <= b.max_corner.x && a.max_corner.x >= b.min_corner.x) &&
           (a.min_corner.y <= b.max_corner.y && a.max_corner.y >= b.min_corner.y) &&
           (a.min_corner.z <= b.max_corner.z && a.max_corner.z >= b.min_corner.z);
}

bool checkModelCollision(const Model& m1, const Model& m2) {
    for (const auto& mesh1 : m1.meshes) {
        AABB a = getTransformedAABB(mesh1, m1.model);

        for (const auto& mesh2 : m2.meshes) {
            AABB b = getTransformedAABB(mesh2, m2.model);

            if (checkAABBCollision(a, b)) {
                return true;
            }
        }
    }
    return false;
}

glm::vec3 getMinimumTranslationVector(const AABB& a, const AABB& b) {
    glm::vec3 mtv(0.0f);

    float dx1 = b.max_corner.x - a.min_corner.x;
    float dx2 = a.max_corner.x - b.min_corner.x;
    float dx = (dx1 < dx2) ? dx1 : -dx2;

    float dy1 = b.max_corner.y - a.min_corner.y;
    float dy2 = a.max_corner.y - b.min_corner.y;
    float dy = (dy1 < dy2) ? dy1 : -dy2;

    float dz1 = b.max_corner.z - a.min_corner.z;
    float dz2 = a.max_corner.z - b.min_corner.z;
    float dz = (dz1 < dz2) ? dz1 : -dz2;

    // Pega o menor eixo de separação
    float minOverlap = std::abs(dx);
    mtv = glm::vec3(dx, 0.0f, 0.0f);

    if (std::abs(dy) < minOverlap) {
        minOverlap = std::abs(dy);
        mtv = glm::vec3(0.0f, dy, 0.0f);
    }

    if (std::abs(dz) < minOverlap) {
        mtv = glm::vec3(0.0f, 0.0f, dz);
    }

    return mtv;
}