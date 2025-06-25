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
#define GRAVITY glm::vec3(0.0f, -200.0f, 0.0f)
#define DAMPING 0.95f
#define CORRECTION_FORCE 1.0f
#define PHYSICS_SCALE 1.0f

using namespace std;

glm::vec3 cameraPositionBase = glm::vec3(0.0f, 0.0f, 200.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

static GLfloat AMPLITUDE = 1.5f;
static GLfloat FREQUENCY = 2.5f;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// camera
Camera camera(cameraPositionBase);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

glm::vec3 cameraQuake(GLfloat time)
{
    GLfloat xoffset = AMPLITUDE * glm::cos(FREQUENCY * time);
    GLfloat yoffset = AMPLITUDE * glm::cos(FREQUENCY * time * 1.1f + 0.5f);
    GLfloat zoffset = AMPLITUDE * glm::cos(FREQUENCY * time * 1.1f);

    return {xoffset, yoffset, zoffset};
}

glm::vec3 modelQuake(GLfloat time)
{
    float baseNoiseX = ((rand() % 1000) / 500.0f - 1.0f); // [-1.0, +1.0]
    float baseNoiseY = ((rand() % 1000) / 500.0f - 1.0f);
    float baseNoiseZ = ((rand() % 1000) / 500.0f - 1.0f);

    // Oscilação suave com seno no tempo
    float sinModX = glm::sin(FREQUENCY * time);
    float sinModY = glm::sin(FREQUENCY * time * 1.1f + 0.5f);
    float sinModZ = glm::sin(FREQUENCY * time * 0.9f + 1.0f);

    // Resultado final mistura ruído com oscilação controlada
    float xOffset = baseNoiseX * sinModX * AMPLITUDE;
    float yOffset = baseNoiseY * sinModY * AMPLITUDE;
    float zOffset = baseNoiseZ * sinModZ * AMPLITUDE;

    return glm::vec3(xOffset, yOffset, zOffset);
}

float randomFloat(float min, float max) {
    float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // Gera um valor aleatório entre 0 e 1
    return min + random * (max - min);  // Escala para o intervalo [min, max]
}

glm::vec3 modelSpeedQuake(GLfloat time) {
    // Aplique AMPLITUDE e FREQUENCY ao movimento do tremor

    // Funções seno para controlar o movimento nos eixos X, Y e Z
    GLfloat xSpeed = AMPLITUDE * glm::sin(FREQUENCY * time);  // Controle de movimento em X
    GLfloat ySpeed = AMPLITUDE * glm::sin(FREQUENCY * time * 1.2f);  // Controle de movimento em Y (ajustando a fase)
    GLfloat zSpeed = AMPLITUDE * glm::sin(FREQUENCY * time * 1.3f);  // Controle de movimento em Z (ajustando a fase)

    // Retorne a velocidade (movimento) do tremor para os modelos
    return glm::vec3(xSpeed, ySpeed, zSpeed);
}

glm::vec3 modelRotationQuake(GLfloat time)
{
    // Gerar rotações aleatórias para os eixos x, y e z
    float xRotation = ((rand() % 1000) / 500.0f - 1.0f) * AMPLITUDE;  // Gera valores entre -AMPLITUDE e +AMPLITUDE
    float yRotation = ((rand() % 1000) / 500.0f - 1.0f) * AMPLITUDE;
    float zRotation = ((rand() % 1000) / 500.0f - 1.0f) * AMPLITUDE;

    // Você pode aplicar uma variação ao longo do tempo, como no caso do deslocamento:
    if ((int)(time * FREQUENCY) % 2 == 0) {
        xRotation *= 1.2f; // Aumenta a rotação em x a cada 2 segundos
    } else {
        yRotation *= 1.5f; // Aumenta a rotação em y a cada 2 segundos
    }

    return glm::vec3(0.0f, yRotation, 0.0f);
}

void applyForce(Model& m, const glm::vec3& force, GLfloat dt) {
    glm::vec3 acceleration = force / m.object.mass;
    m.object.velocity += acceleration * dt;
}

void applyGravity(Model& m, GLfloat dt) {
    // A gravidade afeta a velocidade na direção Y
    glm::vec3 gravityForce = m.object.mass * GRAVITY;  
    glm::vec3 gravityAcceleration = gravityForce / m.object.mass; // A = F/m
    m.object.velocity += gravityAcceleration * dt;  // Aplique a aceleração da gravidade na velocidade
}


int main(int argc, char *argv[])
{
    if (!glfwInit())
    {
        cerr << "Erro inicialicando GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 500, "Projeto Final", nullptr, nullptr);

    if (!window)
    {
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

    Model scene("data/room/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m1("data/table/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m2("data/lampshader/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m3("data/book2/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m4("data/book1/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m5("data/bed/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m6("data/nightstand/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());
    Model m7("data/bulb/source/model.obj", vertexPath.c_str(), fragmentPath.c_str());

    // posicionando elementos
    // ------------------ SALA ------------------
    scene.scale(glm::vec3(50.0f));

    // ------------------ MESA ------------------
    m1.setModelMass(30.0f);
    m1.translate(glm::vec3(-30.0f, -40.0f, -35.0f));
    m1.scale(glm::vec3(20.0f));

    // ------------------ ABAJUR ------------------
    m2.setModelMass(3.0f);
    m2.translate(glm::vec3(-38.0f, -23.0f, -35.0f));
    m2.scale(glm::vec3(7.0f));

    // ------------------ LIVRO 1 ------------------
    m3.setModelMass(0.3f);
    m3.translate(glm::vec3(-20.0f, -27.5f, -34.0f));
    m3.rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0));
    m3.scale(glm::vec3(5.0f));

    // ------------------ CAMA ------------------
    m5.setModelMass(45.0f);
    m5.translate(glm::vec3(32.0f, -35.0f, -20.0f));
    m5.scale(glm::vec3(30.0f, 32.0f, 18.0f));

    // ------------------ CRIADO MUDO ------------------
    m6.setModelMass(15.6);
    m6.translate(glm::vec3(-38.0f, -40.0f, 30.0f));
    m6.rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    m6.scale(glm::vec3(10.0f));

    // ------------------ LIVRO 2 (empilhado) ------------------
    m4.setModelMass(0.5f);
    m4.translate(glm::vec3(-38.0f, -36.5f, 30.0f));
    m4.rotate(30.0f, glm::vec3(0.0f, 1.0f, 0.0));
    m4.scale(glm::vec3(5.0f));

    glm::mat4 view;

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 500.0f,
        0.1f,
        1000.0f);

    models.push_back(m1);
    models.push_back(m2);
    models.push_back(m3);
    models.push_back(m4);
    models.push_back(m5);
    models.push_back(m6);

    bool firstFrame = true;
    AABB scene_AABB = scene.getGlobalAABB();
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        // 1. Cálculo do deslocamento do terremoto
        static glm::vec3 lastShakeOffset(0.0f);
        glm::vec3 currentShakeOffset = modelQuake(currentFrame);
        glm::vec3 shakeDelta = currentShakeOffset - lastShakeOffset;
        lastShakeOffset = currentShakeOffset;

        scene.clearEffect();
        scene.applyEffect(TRANSLATE, currentShakeOffset);

        view = camera.GetViewMatrix();

        glm::vec3 groundVelocity(0.0f);
        if (!firstFrame) {
            groundVelocity = deltaTime > 0.0f ? shakeDelta / deltaTime : glm::vec3(0.0f);
        }

        for (Model &model: models)
        {
            if (!firstFrame) {
                applyGravity(model, deltaTime);

                glm::vec3 shakeForce = groundVelocity * model.object.mass * 0.06f;
                applyForce(model, shakeForce, deltaTime);
                
                glm::vec3 quakeSpeed = modelSpeedQuake(currentFrame);
                // Aplique esse deslocamento no modelo
                model.object.velocity += quakeSpeed * deltaTime * 50.0f; // Aplique a velocidade de tremor
                glm::vec3 modelQuakeDisplacement = model.object.velocity * deltaTime; // Aplique a movimentação

                model.translate(modelQuakeDisplacement);  // Atualize a posição do modelo com a nova velocidade

                // Rotações devido ao tremor
                glm::vec3 quakeRotation = modelRotationQuake(currentFrame);
                model.rotate(quakeRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                // model.rotate(quakeRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

                glm::vec3 torque = quakeRotation * model.object.mass * 0.05f;
                model.object.angularVelocity += torque * deltaTime;

                model.object.angularVelocity *= model.object.angularDamping;
            }

            // Atualizar a rotação com base no torque
            model.rotate(model.object.angularVelocity.y, glm::vec3(0.0f, 1.0f, 0.0f));

            // Aplicar atrito após o movimento
            if (glm::length(model.object.velocity) < 0.01f)
                model.object.velocity = glm::vec3(0.0f);
            else
                model.object.velocity *= 0.95f; // ou outro valor ajustável

            // Aplicar damping no torque
            if (glm::length(model.object.angularVelocity) < 0.01f)
                model.object.angularVelocity = glm::vec3(0.0f);  // Se a rotação for muito baixa, zere
            else
                model.object.angularVelocity *= model.object.angularDamping;  // Aplica o damping para diminuir a rotação


            checkCollisionWithSceneBounds(model, scene_AABB);
        }

        for (int i = 0; i < models.size(); i++)
        {
            for (int j = i + 1; j < models.size(); j++)
            {
                handleModelCollisionPrecise(models[i], models[j]);
            }

            models[i].draw(view, projection, ambient.position, ambient.color, camera.Position);
        }

        scene.draw(view, projection, ambient.position, ambient.color, camera.Position);

        glfwSwapBuffers(window);
        glfwPollEvents();

        firstFrame = false;
    }

    scene.destroy();
    for (Model &model : models)
    {
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
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        AMPLITUDE += 0.1f;
        AMPLITUDE = glm::min(AMPLITUDE, 2.5f);
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        AMPLITUDE -= 0.1f;
        AMPLITUDE = glm::max(AMPLITUDE, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        FREQUENCY += 0.1f;
        FREQUENCY = glm::min(FREQUENCY, 2.5f);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        FREQUENCY -= 0.1f;
        FREQUENCY = glm::max(FREQUENCY, 0.0f);
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
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
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}