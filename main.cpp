#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

using namespace std;
#define OPERATION_SUCCESS 1
#define OPERATION_ERROR 0

struct Face {
    GLuint v[3];
    GLuint vt[3];
    GLuint vn[3];
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Model {
    vector<Vertex> vertices;
    glm::mat4 modelMatrix;
    GLuint VAO;
    GLuint VBO;
};

glm::vec3 calculateNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
    return normal;
}

string readShader(string path) {
    ifstream shaderFile;
    stringstream shaderStream;

    shaderFile.open(path);

    if(!shaderFile.is_open()) {
        return "";
    }

    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    
    return shaderStream.str();
}

int read_obj_attributes(
    string path,
    vector<Vertex>& vertices
) {
    // read the .obj file and store the data in adequate data structures 
    ifstream file(path);

    if (!file) {
        cerr << "Error in open file operation!\n";
        return OPERATION_ERROR;
    }

    string line;
    vector<glm::vec3> temp_vertices, temp_normals;
    vector<Face> faces;

    // the .obj file contain informations about vertices, textures, normals and faces.
    // with the follow format
    //      v x y z -> vertex with coordinates (x,y,z)
    //      vt u v -> texture coordinates (u,v)
    //      vn nx ny nz -> normal (nx,ny,nz)
    //      f -> Face with:
    //          - v1 v2 v3 -> vertices
    //          - v1/vt1 v2/vt2 v3/vt3 -> vertices and textures
    //          - v1//vn1 v2//vn2 v3//vn3 -> vertices and normals
    //          - v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 -> Face with vertices, texture coordinates and normals
    // we don't use texture in this lab

    while (getline(file, line)) {
        stringstream ss(line);
        string type;
        GLfloat x, y, z;

        ss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        } else if (type == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (type == "f") {
            Face face;
            string token;
            int i = 0;

            while (ss >> token && i < 3) {
                // count '/' character in the string
                size_t slash1 = token.find('/');
                size_t slash2 = token.find('/', slash1 + 1);

                if (slash1 == string::npos) {
                    // v1 v2 v3 -> vertices
                    face.v[i] = stoi(token);
                    face.vn[i] = 0;  // Nenhuma normal
                } else if (slash2 == string::npos) {
                    // v1/vt1 v2/vt2 v3/vt3
                    face.v[i] = stoi(token.substr(0, slash1));
                    face.vn[i] = 0;
                } else {
                    // v1//vn1 v2//vn2 v3//vn3
                    // v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
                    face.v[i]  = stoi(token.substr(0, slash1));
                    face.vn[i] = stoi(token.substr(slash2 + 1));
                }
                i++;
            }

            faces.push_back(face);
        }
    }

    for (const Face& face : faces) {
        for (int i = 0; i < 3; ++i) {
            Vertex v;
            v.position = temp_vertices[face.v[i] - 1];
            
            // If not normal, calculate that
            if (face.vn[i] > 0 && face.vn[i] <= temp_normals.size()) {
                v.normal = temp_normals[face.vn[i] - 1];
            } else {
                // Calculating normal from face vertices
                if (i == 2) {
                    glm::vec3 v0 = temp_vertices[face.v[0] - 1];
                    glm::vec3 v1 = temp_vertices[face.v[1] - 1];
                    glm::vec3 v2 = temp_vertices[face.v[2] - 1];
                    v.normal = calculateNormal(v0, v1, v2);
                }
            }

            vertices.push_back(v);
        }
    }

    return OPERATION_SUCCESS;
}

int main(int argc, char* argv[]) {
    vector<string> paths;
    int countImages = 1;

    if(argc > 1) {
        countImages = argc - 1;
        for (int i = 1; i < argc; i++) {
            paths.push_back(argv[i]);
        }
    } else {
        paths.push_back("DNA.obj");
    }

    if(!glfwInit()) {
        cerr << "Erro inicialicando GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 500, "Projeto Final", nullptr, nullptr);

    if(!window) {
        glfwTerminate();
        cout << "Erro ao criar janela" << endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);

    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }

    vector<Model> modelsList;
    
    cout << countImages << endl;
    for (int i = 0; i < countImages; i++) {
        Model m;

        vector<Vertex> vertices;
    
        read_obj_attributes(paths[i], vertices);

        glGenBuffers(1, &m.VBO);
        glGenVertexArrays(1, &m.VAO);

        glBindVertexArray(m.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        
        m.vertices = vertices;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i - 1, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3((i + 1) * pow(0.1, i + 1), (i + 1) * pow(0.1, i + 1), (i + 1) * pow(0.1, i + 1)));

        m.modelMatrix = model;
        modelsList.push_back(m);
    }

    string vertexFileContent = readShader("shaders/vertex.shader");
    string fragmentFileContent = readShader("shaders/fragment.shader");

    const char* vertexShaderSource = vertexFileContent.c_str();
    const char* fragmentShaderSource = fragmentFileContent.c_str();

    GLuint vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    GLint success;
    GLchar infoLog[512];

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "ERROR::VERTEX_SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED\n" << infoLog << endl;
    }

    GLuint shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 2.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 500.0f,
        0.1f,
        100.0f
    );

    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    glm::vec3 viewPos(0.0f, 2.0f, 5.0f);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    GLuint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

    while (!glfwWindowShouldClose(window))  {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& model : modelsList) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model.modelMatrix));
            glBindVertexArray(model.VAO);
            glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
        }

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    
    for (const auto& model : modelsList) {
        glDeleteVertexArrays(1, &model.VAO);
        glDeleteBuffers(1, &model.VBO);
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}