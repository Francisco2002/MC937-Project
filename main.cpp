#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <glm/gtx/string_cast.hpp>
#include "utils/model.hpp"
#include "utils/camera.hpp"
#include "utils/collision.hpp"

#define PI glm::pi<float>()
#define RANDOM 1
#define PHASE 1
#define GRAVITY glm::vec3(0.0f, -9.8f, 0.0f)
#define RESTITUTION 0.4f
#define FRICTION 0.8f
#define DAMPING 0.95f
#define CORRECTION_FORCE 1.0f
#define PHYSICS_SCALE 1.0f

using namespace std;

glm::vec3 cameraPositionBase = glm::vec3(0.0f, 0.0f, 200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

GLfloat AMPLITUDE = 2.5f;
GLfloat FREQUENCY = 25.0f;

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
    GLfloat xoffset = AMPLITUDE * glm::cos(FREQUENCY * time);
    GLfloat yoffset = AMPLITUDE * glm::cos(FREQUENCY * time * 1.1f + 0.5f);
    GLfloat zoffset = AMPLITUDE * glm::cos(FREQUENCY * time * 1.1f);

    return {xoffset, yoffset, zoffset};
}

glm::vec3 modelQuake(GLfloat time) {
    GLfloat xoffset = AMPLITUDE * glm::sin(FREQUENCY * time);
    GLfloat yoffset = AMPLITUDE * glm::sin(FREQUENCY * time * 1.1f + 0.3f);
    GLfloat zoffset = AMPLITUDE * glm::cos(FREQUENCY * time * 1.3f);

    return {xoffset, yoffset, zoffset};
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
    m1.setModelMass(30.0f);
    m1.translate(glm::vec3(-30.0f, -40.0f, -35.0f));
    m = m1.getGlobalAABB();
    // m1.translate(translate_to_inside_room(scene, m));
    m1.scale(glm::vec3(20.0f));

    // ------------------ ABAJUR ------------------
    m2.setModelMass(3.0f);
    m2.translate(glm::vec3(-38.0f, -23.0f, -35.0f));
    m = m2.getGlobalAABB();
    // m2.translate(translate_to_inside_room(scene, m));
    m2.scale(glm::vec3(7.0f));

    // ------------------ LIVRO 1 ------------------
    m3.setModelMass(0.3f);
    m3.translate(glm::vec3(-20.0f, -27.5f, -34.0f));
    m = m3.getGlobalAABB();
    // m3.translate(translate_to_inside_room(scene, m));
    m3.rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0));
    m3.scale(glm::vec3(5.0f));

    // ------------------ LIVRO 2 (empilhado) ------------------
    m4.setModelMass(0.5f);
    m4.translate(glm::vec3(-20.0f, -25.0f, -30.0f));
    m = m4.getGlobalAABB();
    // m4.translate(translate_to_inside_room(scene, m));
    m4.rotate(30.0f, glm::vec3(0.0f, 1.0f, 0.0));
    m4.scale(glm::vec3(5.0f));

    // ------------------ CAMA ------------------
    m5.setModelMass(45.0f);
    m5.translate(glm::vec3(32.0f, -35.0f, -20.0f));
    m = m5.getGlobalAABB();
    // m5.translate(translate_to_inside_room(scene, m));
    m5.scale(glm::vec3(30.0f, 32.0f, 18.0f));

    // ------------------ CRIADO MUDO ------------------
    m6.setModelMass(15.6);
    m6.translate(glm::vec3(-38.0f, -40.0f, 30.0f));
    m = m6.getGlobalAABB();
    // m6.translate(translate_to_inside_room(scene, m));
    m6.rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    m6.scale(glm::vec3(10.0f));

    // ------------------ LÂMPADA ------------------
    m7.setModelMass(0.15);
    m7.translate(glm::vec3(0.0f, 38.0f, 0.0f));
    m = m7.getGlobalAABB();
    // m7.translate(translate_to_inside_room(scene, m));
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

    bool firstUpdate = true;
    while (!glfwWindowShouldClose(window))  {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        view = camera.GetViewMatrix();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 displacement = modelQuake(currentFrame);
        glm::vec3 displacementVelocity(0.0f);
        if(!firstUpdate) {
            glm::vec3 previousDisplacement = modelQuake(currentFrame - deltaTime);
            displacementVelocity = (displacement - previousDisplacement) / deltaTime;
        }

        scenario.clearEffect();
        scenario.applyEffect(TRANSLATE, displacement);

        scenario.draw(view, projection, ambient.position, ambient.color, camera.Position);
        AABB s = scenario.getGlobalAABB();

        for (int i = 0; i< models.size(); i++) {
            Model& model = models[i];
            AABB m_i = model.getGlobalAABB();
            
            // Calculate Forces
            glm::vec3 quakeForce = displacementVelocity * 50.0f;
            glm::vec3 gravityForce = GRAVITY * model.object.mass;

            // Total Acceleration
            glm::vec3 acceleration = (quakeForce + gravityForce) / model.object.mass;

            model.object.velocity += acceleration * deltaTime * PHYSICS_SCALE;

            model.object.velocity *= DAMPING;

            model.object.displacedPosition += model.object.velocity * deltaTime;

            glm::vec3 posEffect = model.object.displacedPosition + m_i.min_corner;

            if (posEffect.y < s.min_corner.y) {
                model.object.displacedPosition.y = glm::abs(m_i.min_corner.y - s.min_corner.y);
            }

            model.clearEffect();
            model.applyEffect(TRANSLATE, model.object.displacedPosition);

            m_i = model.getGlobalAABB();

            // FLOOT COLLISION
            float floorY = s.min_corner.y;
            if (m_i.min_corner.y < floorY) {
                float correctionY = floorY - m_i.min_corner.y;
                model.object.displacedPosition.y += correctionY;

                // Rebote com restituição
                if (model.object.velocity.y < 0.0f) {
                    model.object.velocity.y *= -RESTITUTION;
                    model.object.velocity.x *= FRICTION;
                    model.object.velocity.z *= FRICTION;
                }

                // Reaplica a transformação após correção
                model.clearEffect();
                model.applyEffect(TRANSLATE, model.object.displacedPosition);
                m_i = model.getGlobalAABB(); // atualiza AABB novamente
            }

            // WALL COLLISION
            if(!is_inside(m_i, s)) {
                glm::vec3 correction = translate_to_inside_room(s, m_i);

                if (glm::length(correction) > 0.0f) {
                    model.object.displacedPosition += correction;

                    glm::vec3 impulse = glm::normalize(correction) * CORRECTION_FORCE / model.object.mass;

                    model.object.velocity += impulse;

                    model.clearEffect();
                    model.applyEffect(TRANSLATE, model.object.displacedPosition);
                }
            }

            m_i = model.getGlobalAABB();

            for (size_t j = i + 1; j < models.size(); ++j) {
                Model& b = models[j];
                AABB b_box = b.getGlobalAABB();

                if (checkAABBCollision(m_i, b_box)) {
                    // Vetor mínimo para separação
                    glm::vec3 penetration = getMinimumTranslationVector(m_i, b_box);

                    // Aplica correção dividida entre os dois objetos proporcional à massa
                    float totalMass = model.object.mass + b.object.mass;

                    glm::vec3 correctionA = -(penetration * (b.object.mass / totalMass));
                    glm::vec3 correctionB = (penetration * (model.object.mass / totalMass));

                    model.object.displacedPosition += correctionA;
                    b.object.displacedPosition += correctionB;

                    // Rebote simples: troca parte da velocidade (elástico)
                    glm::vec3 relativeVelocity = b.object.velocity - model.object.velocity;
                    glm::vec3 impulse = relativeVelocity * 0.5f; // 0.5 = coeficiente elástico simplificado

                    model.object.velocity += impulse * (b.object.mass / totalMass);
                    b.object.velocity -= impulse * (model.object.mass / totalMass);

                    // Reaplica transformações
                    model.clearEffect();
                    model.applyEffect(TRANSLATE, model.object.displacedPosition);

                    b.clearEffect();
                    b.applyEffect(TRANSLATE, b.object.displacedPosition);
                }
            }

            model.draw(view, projection, ambient.position, ambient.color, camera.Position);
        }

        glfwSwapBuffers(window);

        glfwPollEvents();

        firstUpdate = false;
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
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        AMPLITUDE += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        AMPLITUDE -= 0.5f;
        AMPLITUDE = glm::max(AMPLITUDE, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        FREQUENCY += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        FREQUENCY -= 0.5f;
        FREQUENCY = glm::max(FREQUENCY, 0.0f);
    }
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