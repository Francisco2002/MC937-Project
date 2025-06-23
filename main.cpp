#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "utils/model.hpp"

using namespace std;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void processInput(GLFWwindow *window) {
    const float cameraSpeed = 0.5f;

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

int main(int argc, char* argv[]) {
    if(!glfwInit()) {
        cerr << "Erro inicialicando GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(800, 500, "Projeto Final", nullptr, nullptr);

    if(!window) {
        glfwTerminate();
        cout << "Erro ao criar janela" << endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Light ambient;
    ambient.position = glm::vec3(0.0f, 30.0f, 0.0f);
    ambient.color = glm::vec3(1.0f); 

    vector<Model> models;
    string vertexPath = "shaders/vertex.shader";
    string fragmentPath = "shaders/fragment.shader";

    Model scenario("data/room/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m1("data/table/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m2("data/lampshader/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m3("data/book2/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m4("data/book1/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m5("data/bed/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m6("data/nightstand/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m7("data/bulb/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());

    /* for(int i = 0; i < m1.meshes[0].vertices.size(); i++) {
        Vertex v = m1.meshes[0].vertices[i];
        cout << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
    } */

    // posicionando elementos
    // ------------------ SALA ------------------
    scenario.scale(glm::vec3(50.0f));

    // ------------------ MESA ------------------
    m1.translate(glm::vec3(-30.0f, -40.0f, -35.0f));
    m1.scale(glm::vec3(20.0f));

    // ------------------ ABAJUR ------------------
    m2.translate(glm::vec3(-38.0f, -23.0f, -35.0f));
    m2.scale(glm::vec3(7.0f));

    // ------------------ LIVRO 1 ------------------
    m3.translate(glm::vec3(-20.0f, -27.5f, -34.0f));
    m3.rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0));
    m3.scale(glm::vec3(5.0f));

    // ------------------ LIVRO 2 (empilhado) ------------------
    m4.translate(glm::vec3(-20.0f, -25.0f, -30.0f));
    m4.rotate(30.0f, glm::vec3(0.0f, 1.0f, 0.0));
    m4.scale(glm::vec3(5.0f));

    // ------------------ CAMA ------------------
    m5.translate(glm::vec3(32.0f, -40.0f, -20.0f));
    m5.scale(glm::vec3(30.0f, 32.0f, 18.0f));

    // ------------------ CRIADO MUDO ------------------
    m6.translate(glm::vec3(-40.0f, -40.0f, 30.0f));
    m6.rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    m6.scale(glm::vec3(10.0f));

    // ------------------ LÂMPADA ------------------
    m7.translate(glm::vec3(0.0f, 40.0f, 0.0f));
    m7.scale(glm::vec3(10.0f));

    models.push_back(m1);
    models.push_back(m2);
    models.push_back(m3);
    models.push_back(m4);
    models.push_back(m5);
    models.push_back(m6);
    models.push_back(m7);

    glm::mat4 view;

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 500.0f,
        0.1f,
        1000.0f
    );
    
    // Variáveis de estado fora do loop
    static float angularVel = 0.0f;
    static float angle = 0.0f;
    glm::vec3 velocity(0.0f);      // velocidade translacional
    glm::vec3 position(0.0f);      // posição acumulada

    while (!glfwWindowShouldClose(window))  {
        processInput(window);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scenario.draw(view, projection, ambient.position, ambient.color);

        for (Model& model : models) {
            model.draw(view, projection, ambient.position, ambient.color, cameraPos);
        }

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    
    for (Model& model : models) {
        model.destroy();
    }

    glfwTerminate();
    return 0;
}