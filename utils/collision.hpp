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

glm::vec3 collideWithRoom(Model& room, Model& model) {
    AABB roomAABB = room.getGlobalAABB();
    AABB modelAABB = model.getGlobalAABB();

    print_corners(roomAABB);
    print_corners(modelAABB);

    glm::vec3 correction(0.0f);

    // Checa cada eixo e calcula a correção necessária para não sair da sala
    if (modelAABB.min_corner.x < roomAABB.min_corner.x) {
        correction.x = roomAABB.min_corner.x - modelAABB.min_corner.x;
    } else if (modelAABB.max_corner.x > roomAABB.max_corner.x) {
        correction.x = roomAABB.max_corner.x - modelAABB.max_corner.x;
    }

    if (modelAABB.min_corner.y < roomAABB.min_corner.y) {
        correction.y = roomAABB.min_corner.y - modelAABB.min_corner.y;
    } else if (modelAABB.max_corner.y > roomAABB.max_corner.y) {
        correction.y = roomAABB.max_corner.y - modelAABB.max_corner.y;
    }

    if (modelAABB.min_corner.z < roomAABB.min_corner.z) {
        correction.z = roomAABB.min_corner.z - modelAABB.min_corner.z;
    } else if (modelAABB.max_corner.z > roomAABB.max_corner.z) {
        correction.z = roomAABB.max_corner.z - modelAABB.max_corner.z;
    }

    return correction;
}


/* AABB getTransformedAABB(const Mesh& mesh, const glm::mat4& modelMatrix) {
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
} */
