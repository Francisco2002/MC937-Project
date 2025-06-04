#ifndef GENERATE_MESH_H
#define GENERATE_MESH_H

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "mesh.hpp"

struct FaceItem {
    GLuint vertexIdx = -1;
    GLuint textureIdx = -1;
    GLuint normalIdx = -1;
};

struct Face {
    std::vector<FaceItem> faceItems;
};

void read_obj_file(
    std::string path,
    std::vector<glm::vec3> &mPositions,
    std::vector<glm::vec2> &mTexCoords,
    std::vector<glm::vec3> &mNormals,
    std::vector<Face> &mFaces
) {
    std::ifstream file(path);
    std::string line;

    if (file.is_open()) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;
        std::vector<Face> faces;

        while (std::getline(file, line)) {
            std::istringstream stream(line);
            std::string type;
            stream >> type;

            if (type == "v") {
                glm::vec3 pos;
                stream >> pos.x >> pos.y >> pos.z;
                positions.push_back(pos);
            } else if (type == "vt") {
                glm::vec2 uv;
                stream >> uv.x >> uv.y;
                texCoords.push_back(uv);
            } else if(type == "vn") {
                glm::vec3 norm;
                stream >> norm.x >> norm.y >> norm.z;
                normals.push_back(norm);
            } else if (type == "f") {
                std::string token;
                Face f;

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

        mPositions = positions;
        mTexCoords = texCoords;
        mNormals = normals;
        mFaces = faces;
    } else {
        std::cerr << "Erro ao abrir o arquivo: " << path << std::endl;
    }
}

Mesh generate_mesh_from_file(std::string path) {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;

    read_obj_file(path, positions, texCoords, normals, faces);

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    for (const auto& face: faces) {
        std::vector<GLuint> indicesTemp;

        for (const auto& fc: face.faceItems) {
            Vertex vertex;

            GLuint vidx = fc.vertexIdx, tidx = fc.textureIdx, nidx = fc.normalIdx; 
            

            glm::vec3 position = positions[vidx];
            glm::vec2 texCoord = tidx == -1 ? glm::vec2(0.0f) : texCoords[tidx];
            glm::vec3 normal = nidx == -1 ? glm::vec3(0.0f) : normals[nidx];

            vertex.position = position;
            vertex.textureCoord = texCoord;
            vertex.normal = normal;

            vertices.push_back(vertex);
            indicesTemp.push_back(vertices.size() - 1);
        }

        GLuint n = indicesTemp.size();

        for (int i = 1; i < n - 1; ++i) {
            indices.push_back(indicesTemp[0]);
            indices.push_back(indicesTemp[i]);
            indices.push_back(indicesTemp[i + 1]);
        }
    }

    Mesh mesh(vertices, indices);
    
    return mesh;
}

#endif