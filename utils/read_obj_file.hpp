#ifndef GENERATE_MESH_H
#define GENERATE_MESH_H

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "mesh.hpp"

struct FaceItem {
    GLuint vertexIdx = -1;
    GLuint textureIdx = -1;
    GLuint normalIdx = -1;
};

struct Face {
    std::vector<FaceItem> faceItems;
    std::string materialName;
    std::string groupName;
};

void read_mtl_file(
    const std::string& path,
    std::unordered_map<std::string, Material>& materialMap
) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir arquivo MTL: " << path << std::endl;
        return;
    }

    std::string line;
    std::string currentMaterial;
    Material material;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "newmtl") {
            if (!currentMaterial.empty()) {
                materialMap[currentMaterial] = material;
            }
            stream >> currentMaterial;
            material = Material(); // Reseta para o novo material
        } else if (type == "Ka") {
            stream >> material.ambient.r >> material.ambient.g >> material.ambient.b;
        } else if (type == "Kd") {
            stream >> material.diffuse.r >> material.diffuse.g >> material.diffuse.b;
        } else if (type == "Ks") {
            stream >> material.specular.r >> material.specular.g >> material.specular.b;
        } else if (type == "map_Kd" || type == "map_Ns" || type == "map_Bump") {
            std::string token;
            bool texturePathSet = false;

            // Variáveis temporárias para opções (pode guardar se quiser usar)
            glm::vec3 scale(1.0f);
            glm::vec3 offset(0.0f);
            float bumpMultiplier = 1.0f;

            while (stream >> token) {
                if (token == "-s") {
                    stream >> scale.x >> scale.y >> scale.z;
                } else if (token == "-o") {
                    stream >> offset.x >> offset.y >> offset.z;
                } else if (token == "-bm") {
                    std::string bmStr;
                    stream >> bmStr;
                    std::replace(bmStr.begin(), bmStr.end(), ',', '.');
                    bumpMultiplier = std::stof(bmStr);
                } else {
                    // token que não é flag: deve ser caminho da textura
                    // Se já setou antes, ignore o restante (ou trate erro)
                    if (!texturePathSet) {
                        // salvar caminho na struct do material conforme map_*
                        if (type == "map_Kd") material.diffuseTexturePath = token;
                        else if (type == "map_Ns") material.specularTexturePath = token;
                        else if (type == "map_Bump") {
                            material.bumpTexturePath = token;
                        }
                        texturePathSet = true;
                    }
                }
            }
        }
    }

    if (!currentMaterial.empty()) {
        materialMap[currentMaterial] = material;
    }
}

void read_obj_file(
    std::string path,
    std::vector<glm::vec3> &mPositions,
    std::vector<glm::vec2> &mTexCoords,
    std::vector<glm::vec3> &mNormals,
    std::vector<Face> &mFaces,
    std::unordered_map<std::string, Material> &mMaterials
) {
    std::ifstream file(path);
    std::string line;

    if (file.is_open()) {
        std::string currentMaterial = "default";
        std::string currentGroup = "default";
        
        std::unordered_map<std::string, Material> materialMap;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;
        std::vector<Face> faces;

        while (std::getline(file, line)) {
            std::istringstream stream(line);
            std::string type;
            stream >> type;

            if (type == "mtllib") {
                std::string mtlFilename;
                stream >> mtlFilename;

                fs::path obj_fs_path(path);
                fs::path mtl_fs_path = obj_fs_path.parent_path() / mtlFilename; // ajuste conforme o nome do arquivo mtl


                read_mtl_file(mtl_fs_path.string(), materialMap);
            } else if (type == "usemtl") {
                stream >> currentMaterial;
                if (currentMaterial.empty()) currentMaterial = "default";
            } else if (type == "g" || type == "o") {
                stream >> currentGroup;
                if (currentGroup.empty()) currentGroup = "default";
            } else if (type == "v") {
                glm::vec3 pos;
                stream >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            } else if (type == "vt") {
                glm::vec2 uv;
                stream >> uv.x >> uv.y;
                uv.y = 1.0f - uv.y; // Corrige inversão vertical
                texCoords.push_back(uv);
            } else if(type == "vn") {
                glm::vec3 norm;
                stream >> norm.x >> norm.y >> norm.z;
                normals.push_back(norm);
            } else if (type == "f") {
                std::string token;
                Face f;
                f.materialName = currentMaterial;
                f.groupName = currentGroup;

                while (stream >> token) {
                    size_t slash1 = token.find('/');
                    size_t slash2 = token.find('/', slash1 + 1);

                    int idxVertex = 0, idxTex = -1, idxNormal = -1;

                    if (slash1 == std::string::npos) {
                        // f v1 v2 v3
                        idxVertex = stoi(token) - 1;
                    } else if (slash2 == std::string::npos) {
                        // f v1/vt1
                        idxVertex = stoi(token.substr(0, slash1)) - 1;
                        idxTex = stoi(token.substr(slash1 + 1)) - 1;
                    } else {
                        // f v1/vt1/vn1
                        idxVertex = stoi(token.substr(0, slash1)) - 1;

                        if (slash1 + 1 < slash2) {
                            idxTex = stoi(token.substr(slash1 + 1, slash2 - slash1 - 1)) - 1;
                        }
                        
                        idxNormal = stoi(token.substr(slash2 + 1)) - 1;
                    }
                
                    FaceItem fc;
                    fc.vertexIdx = idxVertex;
                    fc.textureIdx = idxTex;
                    fc.normalIdx = idxNormal;

                    f.faceItems.push_back(fc);
                }

                faces.push_back(f);
            }
        }

        std::vector<glm::vec3> positionsNormalized;
        glm::vec3 min = positions[0];
        glm::vec3 max = positions[0];

        for(const auto& p: positions) {
            min = glm::min(min, p);
            max = glm::max(max, p);
        }

        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 size = max - min;
        float maxDimension = std::max({size.x, size.y, size.z});

        for (const auto& p : positions) {
            glm::vec3 n = (p - center) * (2.0f / maxDimension); // escala uniforme
            positionsNormalized.push_back(n);
        }

        mPositions = positionsNormalized;
        mTexCoords = texCoords;
        mNormals = normals;
        mFaces = faces;
        mMaterials = materialMap;
    } else {
        std::cerr << "Erro ao abrir o arquivo: " << path << std::endl;
    }
}

// Novo tipo para chave composta
struct MeshKey {
    std::string material;
    std::string group;

    std::string get_key() const {
        return group + "::" + material;
    }

    bool operator==(const MeshKey& other) const {
        return material == other.material && group == other.group;
    }
};

namespace std {
    template <>
    struct hash<MeshKey> {
        std::size_t operator()(const MeshKey& k) const {
            return hash<std::string>()(k.material) ^
                   (hash<std::string>()(k.group) << 1);
        }
    };
}

std::vector<Mesh> generate_mesh_from_file(std::string path) {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;
    std::unordered_map<std::string, Material> materials;

    std::string baseDir = fs::path(path).parent_path().string();
    read_obj_file(path, positions, texCoords, normals, faces, materials);

    // Agrupar faces por material
    std::unordered_map<MeshKey, std::vector<Face>> facesByKey;
    for (const auto& face : faces) {
        MeshKey key = {face.materialName, face.groupName};
        facesByKey[key].push_back(face);
    }

    std::vector<Mesh> meshes;

    for (const auto& [key, faceGroup] : facesByKey) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        std::unordered_map<std::string, GLuint> uniqueVertices; // evitar duplicatas

        for (const auto& face : faceGroup) {
            std::vector<GLuint> indicesTemp;

            for (const auto& fc : face.faceItems) {
                Vertex vertex;

                glm::vec3 position = positions[fc.vertexIdx];
                glm::vec2 texCoord = (fc.textureIdx == -1) ? glm::vec2(0.0f) : texCoords[fc.textureIdx];
                glm::vec3 normal = (fc.normalIdx == -1) ? glm::vec3(0.0f) : normals[fc.normalIdx];

                vertex.position = position;
                vertex.textureCoord = texCoord;
                vertex.normal = normal;

                vertices.push_back(vertex);
                indicesTemp.push_back(vertices.size() - 1);
            }

            for (size_t i = 1; i + 1 < indicesTemp.size(); ++i) {
                indices.push_back(indicesTemp[0]);
                indices.push_back(indicesTemp[i]);
                indices.push_back(indicesTemp[i + 1]);
            }
        }

        Material mat = materials.count(key.material) ? materials[key.material] : Material();
        
        std::string mesh_name = key.get_key();
        meshes.emplace_back(mesh_name, vertices, indices, mat, baseDir);
    }

    return meshes;
}

#endif