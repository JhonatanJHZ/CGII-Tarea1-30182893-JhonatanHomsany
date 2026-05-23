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

#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

Application::Application()
    : glfwManager(nullptr), uiManager(nullptr), renderer(nullptr),
      shader(nullptr), scene(nullptr), camera(nullptr), lighting(nullptr) {}

Application::~Application() {
    cleanup();
}

bool Application::init() {
    glfwManager = new GLFWManager();
    this->window = glfwManager->createWindow(800, 1000, "Tarea 1 - Carga de Escena GLTF/GLB"); 
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
    ray = new Ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 0.0f, -1.0f), Color{1.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 10.0f);
    picker = new InputPicker();

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
    renderer->clear();
    uiManager->newFrame();
    uiManager->drawInspector(scene, lighting, ray, picker);

    glm::mat4 view = camera->getViewMatrix();

    int display_w, display_h;
    glfwManager->getFrameBufferSize(&display_w, &display_h);
    display_h = std::max(1, display_h);
    float aspect = (float)display_w / (float)display_h;
    glm::mat4 projection = camera->getProjectionMatrix(aspect);

    scene->draw(shader, renderer, view, projection, lighting, camera->getPosition());

    ray->hit_object = nullptr;
    ray->hit_t = ray->t_max;
    for(SceneObject &obj : scene->objects){
        ray->intersect(&obj);
    }
    ray->drawRay(view, projection, flatShader);

    ImGuiIO& io = ImGui::GetIO();
    if (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !io.WantCaptureMouse) {
        double mouseX, mouseY;
        glfwGetCursorPos(this->window, &mouseX, &mouseY);
        picker->pick(mouseX, mouseY, display_w, display_h, view, projection, scene);
    }

    uiManager->render();
    glfwManager->update();
}



void Application::cleanup() {
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
