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

    vector<Model> models;
    string vertexPath = "shaders/vertex.shader";
    string fragmentPath = "shaders/fragment.shader";

    int i = 0;
    for(string path: paths) {
        Model m(path, vertexPath.c_str(), fragmentPath.c_str());

        float angle = i * (360.0f / countImages);

        float x = cos(glm::radians(angle)) * 10.0f;
        float z = sin(glm::radians(angle)) * 10.0f;

        m.scale(glm::vec3(0.5f));
        m.translate(glm::vec3(x, 0.0f, z));
        models.push_back(m);

        i++;
    }

    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 2.0f, 50.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 500.0f,
        0.1f,
        1000.0f
    );

    glm::vec3 lightPos(100.0f);
    glm::vec3 viewPos(0.0f, 2.0f, 50.0f);

    
    while (!glfwWindowShouldClose(window))  {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (Model model : models) {
            model.draw(view, projection, lightPos, viewPos);
        }

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    
    for (Model model : models) {
        model.destroy();
    }

    glfwTerminate();
    return 0;
}