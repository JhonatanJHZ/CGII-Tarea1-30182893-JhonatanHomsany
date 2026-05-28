#include "../include/Application.h"
#include "../include/tools/GLFWManager.h"
#include "../include/tools/UIManager.h"
#include "../include/Renderer.h"
#include "../include/Shader.h"
#include "../include/Camera.h"
#include "../include/tools/GLTFManager.h"
#include "../include/Scene.h"
#include "../include/Lighting.h"
#include "../include/Ray.h"
#include "../include/tools/InputPicker.h"
#include "../include/tools/BasicShapesGenerator.h"
#include "../include/tools/ShadowManager.h"


#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

void drawSelectionBox(const SceneObject* obj, const glm::mat4& view, const glm::mat4& projection, Shader* shader) {
    if (!obj || !shader) return;
    glm::vec3 c[8] = {
        glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f, -1.0f),
        glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(-1.0f,  1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 1.0f, -1.0f,  1.0f),
        glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f)
    };

    float lineVertices[] = {
        c[0].x, c[0].y, c[0].z, c[1].x, c[1].y, c[1].z,
        c[1].x, c[1].y, c[1].z, c[2].x, c[2].y, c[2].z,
        c[2].x, c[2].y, c[2].z, c[3].x, c[3].y, c[3].z,
        c[3].x, c[3].y, c[3].z, c[0].x, c[0].y, c[0].z,

        c[4].x, c[4].y, c[4].z, c[5].x, c[5].y, c[5].z,
        c[5].x, c[5].y, c[5].z, c[6].x, c[6].y, c[6].z,
        c[6].x, c[6].y, c[6].z, c[7].x, c[7].y, c[7].z,
        c[7].x, c[7].y, c[7].z, c[4].x, c[4].y, c[4].z,

        c[0].x, c[0].y, c[0].z, c[4].x, c[4].y, c[4].z,
        c[1].x, c[1].y, c[1].z, c[5].x, c[5].y, c[5].z,
        c[2].x, c[2].y, c[2].z, c[6].x, c[6].y, c[6].z,
        c[3].x, c[3].y, c[3].z, c[7].x, c[7].y, c[7].z
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shader->ID);
    
    glm::mat4 modelMatrix = obj->getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    glUniform4f(glGetUniformLocation(shader->ID, "objectColor"), 0.0f, 1.0f, 0.0f, 1.0f);

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 24);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

Application::Application()
    : glfwManager(nullptr), uiManager(nullptr), renderer(nullptr),
      shader(nullptr), scene(nullptr), camera(nullptr), lighting(nullptr) {}

Application::~Application() {
    cleanup();
}

bool Application::init() {
    glfwManager = new GLFWManager();
    this->window = glfwManager->createWindow(800, 1000, "Tarea 1 - 30182893 - Jhonatan Homsany"); 
    if (!this->window) {
        return false;
    }
    renderer = new Renderer();
    shader = new Shader("../../../Tarea1-30182893-JhonatanHomsany/shaders/default.vert", "../../../Tarea1-30182893-JhonatanHomsany/shaders/default.frag");
    flatShader = new Shader("../../../Tarea1-30182893-JhonatanHomsany/shaders/flat.vert", "../../../Tarea1-30182893-JhonatanHomsany/shaders/flat.frag");
    camera = new Camera();
    uiManager = new UIManager(window);

    scene = new Scene();
    lighting = new Lighting();
    ray = new Ray(glm::vec3(-2.45f, -1.8f, -0.9f), glm::vec3(1.65f, -1.45f, 0.2f), Color{32.0f/255.0f, 63.0f/255.0f, 185.0f/255.0f, 1.0f}, 0.0f, 10.0f);
    picker = new InputPicker();

    ShadowManager::initShadowFBO();
    loadBoxScene();

    return true;
}

void Application::run() {
    if (!init()) {
        return;
    }

    while (!glfwManager->shouldClose()) {
        updateAndRender();
    }
}

void Application::updateAndRender() {
    static float lastFrame = 0.0f;
    float currentFrame = (float)glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    renderer->clear();
    uiManager->newFrame();
    uiManager->drawInspector(scene, lighting, ray, picker, camera);

    glm::mat4 view = camera->getViewMatrix();

    int display_w, display_h;
    glfwManager->getFrameBufferSize(&display_w, &display_h);
    display_h = std::max(1, display_h);
    float aspect = (float)display_w / (float)display_h;
    glm::mat4 projection = camera->getProjectionMatrix(aspect);

    if (ShadowManager::mode == ShadowMode::PLANAR) {
        if (lighting && !lighting->lights.empty()) {
            glm::vec3 lightPos = lighting->lights[0].position;
            ShadowManager::renderPlanarShadows(scene, flatShader, renderer, lightPos, -7.99f, view, projection);
        }
    } else if (ShadowManager::mode == ShadowMode::SHADOW_MAPPING) {
        // ... Ejecutar la lógica de Shadow Mapping ...
    }

    scene->draw(shader, renderer, view, projection, lighting, camera->getPosition());

    ray->hit_object = nullptr;
    ray->hit_t = ray->t_max;
    for(SceneObject &obj : scene->objects){
        ray->intersect(&obj);
    }
    ray->drawRay(view, projection, flatShader);
    if(picker && picker->hit_object){
        drawSelectionBox(picker->hit_object, view, projection, flatShader);
    }

    ImGuiIO& io = ImGui::GetIO();
    if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !io.WantCaptureMouse) {
        double mouseX, mouseY;
        glfwGetCursorPos(this->window, &mouseX, &mouseY);
        picker->pick(mouseX, mouseY, display_w, display_h, view, projection, scene);
    }

    handleKeyboardEvents(deltaTime);

    uiManager->render();
    glfwManager->update();
}

void Application::loadBoxScene() {
    BasicShapesGenerator::loadDefaultBoxScene(scene, lighting, camera);
}

void Application::handleKeyboardEvents(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    
    if (!io.WantCaptureKeyboard) {
        if (glfwGetKey(this->window, GLFW_KEY_W) == GLFW_PRESS) {
            camera->movement(MovementDirection::FORWARD, deltaTime);
        }
        if (glfwGetKey(this->window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->movement(MovementDirection::BACKWARD, deltaTime);
        }
        if (glfwGetKey(this->window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->movement(MovementDirection::LEFT, deltaTime);
        }
        if (glfwGetKey(this->window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->movement(MovementDirection::RIGHT, deltaTime);
        }
    }

    static bool isMouseCaptured = false;
    static double lastMouseX = 0.0;
    static double lastMouseY = 0.0;

    if (glfwGetKey(this->window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (!isMouseCaptured) {
            glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(this->window, &lastMouseX, &lastMouseY);
            isMouseCaptured = true;
        } else {
            double mouseX, mouseY;
            glfwGetCursorPos(this->window, &mouseX, &mouseY);
            
            float xOffset = (float)(mouseX - lastMouseX);
            float yOffset = (float)(lastMouseY - mouseY);

            lastMouseX = mouseX;
            lastMouseY = mouseY;
            camera->rotate(xOffset, yOffset);
        }
    } else {
        if (isMouseCaptured) {
            glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isMouseCaptured = false;
        }
    }
}



void Application::cleanup() {
    ShadowManager::cleanupShadowFBO();
    delete uiManager;
    uiManager = nullptr;
    delete camera;
    camera = nullptr;
    delete scene;
    scene = nullptr;
    delete lighting;
    lighting = nullptr;
    delete shader;
    shader = nullptr;
    delete flatShader;
    flatShader = nullptr;
    delete renderer;
    renderer = nullptr;
    delete glfwManager;
    glfwManager = nullptr;
    delete ray;
    ray = nullptr;
    delete picker;
    picker = nullptr;
}
