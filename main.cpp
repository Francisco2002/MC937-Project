#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "utils/model.hpp"
#include "utils/camera.hpp"
#include "utils/collision.hpp"

#define PI glm::pi<float>()
#define AMPLITUDE 6.0f
#define FREQUENCY 25.0f
#define RANDOM 1
#define PHASE 1

using namespace std;

glm::vec3 cameraPositionBase = glm::vec3(0.0f, 0.0f, 200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// camera
Camera camera(cameraPositionBase);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

glm::vec3 cameraQuake(GLfloat time) {
    GLfloat xoffset = AMPLITUDE * glm::sin(FREQUENCY * time);
    GLfloat yoffset = AMPLITUDE * glm::sin(FREQUENCY * time * 1.1f + 0.5f);
    GLfloat zoffset = AMPLITUDE * glm::sin(FREQUENCY * time * 1.1f);

    return {xoffset, yoffset, zoffset};
}

glm::vec3 modelQuake(GLfloat time) {
    GLfloat xoffset = AMPLITUDE * glm::sin(FREQUENCY * time);
    GLfloat yoffset = AMPLITUDE * glm::sin(FREQUENCY * time * 1.1f + 0.3f);
    GLfloat zoffset = AMPLITUDE * glm::cos(FREQUENCY * time * 1.3f);

    return {xoffset, yoffset, 0.0f};
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

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


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

    // posicionando elementos
    // ------------------ SALA ------------------
    scenario.scale(glm::vec3(50.0f));
    AABB scene = scenario.getGlobalAABB();

    AABB m;
    
    // ------------------ MESA ------------------
    m1.translate(glm::vec3(-30.0f, -40.0f, -35.0f));
    m = m1.getGlobalAABB();
    m1.translate(translate_to_inside_room(scene, m));
    m1.scale(glm::vec3(20.0f));

    // ------------------ ABAJUR ------------------
    m2.translate(glm::vec3(-38.0f, -23.0f, -35.0f));
    m = m2.getGlobalAABB();
    m2.translate(translate_to_inside_room(scene, m));
    m2.scale(glm::vec3(7.0f));

    // ------------------ LIVRO 1 ------------------
    m3.translate(glm::vec3(-20.0f, -27.5f, -34.0f));
    m = m3.getGlobalAABB();
    m3.translate(translate_to_inside_room(scene, m));
    m3.rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0));
    m3.scale(glm::vec3(5.0f));

    // ------------------ LIVRO 2 (empilhado) ------------------
    m4.translate(glm::vec3(-20.0f, -25.0f, -30.0f));
    m = m4.getGlobalAABB();
    m4.translate(translate_to_inside_room(scene, m));
    m4.rotate(30.0f, glm::vec3(0.0f, 1.0f, 0.0));
    m4.scale(glm::vec3(5.0f));

    // ------------------ CAMA ------------------
    m5.translate(glm::vec3(32.0f, -35.0f, -20.0f));
    m = m5.getGlobalAABB();
    m5.translate(translate_to_inside_room(scene, m));
    m5.scale(glm::vec3(30.0f, 32.0f, 18.0f));

    // ------------------ CRIADO MUDO ------------------
    m6.translate(glm::vec3(-38.0f, -40.0f, 30.0f));
    m = m6.getGlobalAABB();
    m6.translate(translate_to_inside_room(scene, m));
    m6.rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    m6.scale(glm::vec3(10.0f));

    // ------------------ LÂMPADA ------------------
    m7.translate(glm::vec3(0.0f, 38.0f, 0.0f));
    m = m7.getGlobalAABB();
    m7.translate(translate_to_inside_room(scene, m));
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

    while (!glfwWindowShouldClose(window))  {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        view = camera.GetViewMatrix();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 displacement = modelQuake(currentFrame);

        scenario.clearEffect();
        scenario.applyEffect(TRANSLATE, displacement);

        scenario.draw(view, projection, ambient.position, ambient.color, camera.Position);

        AABB s = scenario.getGlobalAABB();
        
        for (Model& model : models) {
            AABB m_i = model.getGlobalAABB();

            if(!is_inside(m_i, s)) {
                glm::vec3 correction = translate_to_inside_room(s, m_i);

                if (glm::length(correction) > 0.0f) {
                    model.translate(correction);
                }
            }

            model.draw(view, projection, ambient.position, ambient.color, camera.Position, true);
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

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}