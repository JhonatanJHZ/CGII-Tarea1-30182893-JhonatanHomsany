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
#include "../include/Mesh.h"
#include "../include/tools/BasicShapesGenerator.h"
#include "../include/tools/ShadowManager.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
void drawSelectionBox(const SceneObject* obj, const glm::mat4& view, const glm::mat4& projection, Shader* shader) {
    if (!obj || !shader) return;
    glm::vec3 minAABB(-0.5f, -0.5f, -0.5f);
    glm::vec3 maxAABB(0.5f, 0.5f, 0.5f);
    std::vector<Vertex> localVertices;
    if (obj->type == MeshType::REVOLUTION_SOLID && obj->meshPointer) {
        localVertices = static_cast<Mesh*>(obj->meshPointer)->getVertices();
    } else if (obj->type == MeshType::GLTF && obj->meshPointer) {
        localVertices = static_cast<GLTFManager*>(obj->meshPointer)->getVertices();
    }
    if (!localVertices.empty()) {
        minAABB = localVertices[0].position;
        maxAABB = localVertices[0].position;
        for (const auto& v : localVertices) {
            minAABB = glm::min(minAABB, v.position);
            maxAABB = glm::max(maxAABB, v.position);
        }
    } else if (obj->shape == ShapeType::PLANE) {
        minAABB = glm::vec3(-1.0f, 0.0f, -1.0f);
        maxAABB = glm::vec3(1.0f, 0.0f, 1.0f);
    } else if (obj->shape == ShapeType::SPHERE) {
        minAABB = glm::vec3(-0.5f, -0.5f, -0.5f);
        maxAABB = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    glm::vec3 c[8] = {
        glm::vec3(minAABB.x, minAABB.y, minAABB.z), glm::vec3(maxAABB.x, minAABB.y, minAABB.z),
        glm::vec3(maxAABB.x, maxAABB.y, minAABB.z), glm::vec3(minAABB.x, maxAABB.y, minAABB.z),
        glm::vec3(minAABB.x, minAABB.y, maxAABB.z), glm::vec3(maxAABB.x, minAABB.y, maxAABB.z),
        glm::vec3(maxAABB.x, maxAABB.y, maxAABB.z), glm::vec3(minAABB.x, maxAABB.y, maxAABB.z)
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
      shader(nullptr), scene(nullptr), lighting(nullptr) {}
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
    shadowDepthShader = new Shader("../../../Tarea1-30182893-JhonatanHomsany/shaders/shadowDepth.vert", "../../../Tarea1-30182893-JhonatanHomsany/shaders/shadowDepth.frag");
    cameras.push_back(new Camera());
    activeCameraIndex = 0;
    uiManager = new UIManager(window);
    scene = new Scene();
    lighting = new Lighting();
    ray = new Ray(glm::vec3(-2.45f, -1.8f, -0.9f), glm::vec3(0.75f, -1.45f, 0.2f), Color{0.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f}, 0.0f, 10.0f);
    picker = new InputPicker();
    float quadVertices[] = {
        -1.0f,  1.0f, 
        -1.0f, -1.0f, 
         1.0f, -1.0f, 
        -1.0f,  1.0f, 
         1.0f, -1.0f, 
         1.0f,  1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    raytraceShader = new Shader("../../../Tarea1-30182893-JhonatanHomsany/shaders/raytrace.vert", "../../../Tarea1-30182893-JhonatanHomsany/shaders/raytrace.frag");
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
    Camera* activeCamera = cameras[activeCameraIndex];
    renderer->clear();
    uiManager->newFrame();
    uiManager->drawInspector(scene, lighting, ray, picker, cameras, activeCameraIndex);
    int display_w, display_h;
    glfwManager->getFrameBufferSize(&display_w, &display_h);
    display_h = std::max(1, display_h);
    float aspect = (float)display_w / (float)display_h;
    glm::mat4 view = activeCamera->getViewMatrix();
    glm::mat4 projection = activeCamera->getProjectionMatrix(aspect);
    if (activeCamera->getRenderMode() == RenderMode::RASTERIZATION) {
        glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
        if (lighting && !lighting->lights.empty() && ShadowManager::mode == ShadowMode::SHADOW_MAPPING) {
            glm::vec3 lightPos = lighting->lights[0].position;
            lightSpaceMatrix = ShadowManager::renderShadowMap(scene, shadowDepthShader, renderer, lightPos);
            glViewport(0, 0, display_w, display_h);
        }
        glUseProgram(shader->ID);
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        glUniform1i(glGetUniformLocation(shader->ID, "shadowMode"), static_cast<int>(ShadowManager::mode));
        glUniform1i(glGetUniformLocation(shader->ID, "showOnlyShadows"), static_cast<int>(ShadowManager::showOnlyShadows));
        glUniform1f(glGetUniformLocation(shader->ID, "biasForShadowMapping"), ShadowManager::biasForShadowMapping); 
        glUniform1i(glGetUniformLocation(shader->ID, "useAdaptativeBias"), static_cast<int>(ShadowManager::useAdaptativeBias));
        glUniform1i(glGetUniformLocation(shader->ID, "usePCF"), static_cast<int>(ShadowManager::usePCF));
        glUniform1i(glGetUniformLocation(shader->ID, "pcfKernelRadius"), ShadowManager::pcfKernelRadius); 
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ShadowManager::depthMapTexture);
        glUniform1i(glGetUniformLocation(shader->ID, "shadowMap"), 1); 
        scene->draw(shader, renderer, view, projection, lighting, activeCamera->getPosition());
        if (lighting && !lighting->lights.empty()) {
            if (ShadowManager::mode == ShadowMode::PLANAR) {
                glm::vec3 lightPos = lighting->lights[0].position;
                ShadowManager::renderPlanarShadows(scene, flatShader, renderer, lightPos, -8.0f, view, projection);
            }
        }
        if (ShadowManager::mode == ShadowMode::SHADOW_MAPPING && ShadowManager::showDepthMap) {
            ImGui::Begin("Mapa de Profundidad (FBO)");
            ImVec2 size = ImVec2(256.0f, 256.0f);
            ImGui::Image((ImTextureID)(uintptr_t)ShadowManager::depthMapTexture, size, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
        }
    } else if(activeCamera->getRenderMode() == RenderMode::RAY_TRACING) {
        glUseProgram(raytraceShader->ID);
        glm::vec3 camPos = activeCamera->getPosition();
        glm::vec3 front = glm::normalize(activeCamera->getTarget() - camPos);
        glm::vec3 right = glm::normalize(glm::cross(front, activeCamera->getUp()));
        glm::vec3 up = glm::cross(right, front);
        float fov = activeCamera->getFov();
        glUniform3fv(glGetUniformLocation(raytraceShader->ID, "camPos"), 1, glm::value_ptr(camPos));
        glUniform3fv(glGetUniformLocation(raytraceShader->ID, "camFront"), 1, glm::value_ptr(front));
        glUniform3fv(glGetUniformLocation(raytraceShader->ID, "camUp"), 1, glm::value_ptr(up));
        glUniform3fv(glGetUniformLocation(raytraceShader->ID, "camRight"), 1, glm::value_ptr(right));
        glUniform1f(glGetUniformLocation(raytraceShader->ID, "fov"), fov);
        glUniform1f(glGetUniformLocation(raytraceShader->ID, "aspect"), aspect);
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "maxBounces"), activeCamera->getRayBounces());
        int lightCount = 0;
        for (Light& light : lighting->lights) {
            if(lightCount >= 10) break;
            std::string base = "lights[" + std::to_string(lightCount) + "]";
            glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".position").c_str()), 1, glm::value_ptr(light.position));
            glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(light.color));
            glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".intensity").c_str()), light.intensity);
            glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".ambientIntensity").c_str()), light.ambientIntensity);
            lightCount++;
        }
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numLights"), lightCount);
        int sphereCount = 0;
        for (SceneObject& obj : scene->objects) {
            if (obj.shape == ShapeType::SPHERE && sphereCount < 50) {
                std::string base = "spheres[" + std::to_string(sphereCount) + "]";
                float realRadius = obj.scale.x * 0.5f;
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".center").c_str()), 1, glm::value_ptr(obj.position));
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".radius").c_str()), realRadius);
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(obj.color));
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".reflectivity").c_str()), obj.reflectivity);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".transparency").c_str()), obj.transparency);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".refractiveIndex").c_str()), obj.refractiveIndex);
                sphereCount++;
            }
        }
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numSpheres"), sphereCount);
        int planeCount = 0;
        for (SceneObject& obj : scene->objects) {
            if (obj.shape == ShapeType::PLANE && planeCount < 10) {
                std::string base = "planes[" + std::to_string(planeCount) + "]";
                glm::mat4 model = obj.getModelMatrix();
                glm::vec3 normal = glm::normalize(glm::vec3(model * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".point").c_str()), 1, glm::value_ptr(obj.position));
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".normal").c_str()), 1, glm::value_ptr(normal));
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(obj.color));
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".reflectivity").c_str()), obj.reflectivity);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".transparency").c_str()), obj.transparency);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".refractiveIndex").c_str()), obj.refractiveIndex);
                planeCount++;
            }
        }
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numPlanes"), planeCount);
        int triCount = 0;
        for (SceneObject& obj : scene->objects) {
            if (obj.shape == ShapeType::NONE && obj.type == MeshType::REVOLUTION_SOLID && obj.meshPointer != nullptr) {
                glm::mat4 model = obj.getModelMatrix();
                const auto& vertices = static_cast<Mesh*>(obj.meshPointer)->getVertices();
                for (size_t i = 0; i < vertices.size() && triCount < 50; i += 3) {
                    glm::vec3 v0 = glm::vec3(model * glm::vec4(vertices[i].position, 1.0f));
                    glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[i+1].position, 1.0f));
                    glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[i+2].position, 1.0f));
                    glm::vec3 edge1 = v1 - v0;
                    glm::vec3 edge2 = v2 - v0;
                    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
                    std::string base = "triangles[" + std::to_string(triCount) + "]";
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".v0").c_str()), 1, glm::value_ptr(v0));
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".v1").c_str()), 1, glm::value_ptr(v1));
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".v2").c_str()), 1, glm::value_ptr(v2));
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".normal").c_str()), 1, glm::value_ptr(normal));
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(obj.color));
                    glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".reflectivity").c_str()), obj.reflectivity);
                    glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".transparency").c_str()), obj.transparency);
                    glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".refractiveIndex").c_str()), obj.refractiveIndex);
                    triCount++;
                }
            }
        }
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numTriangles"), triCount);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    ray->hit_object = nullptr;
    ray->hit_t = ray->t_max;
    for(SceneObject &obj : scene->objects){
        ray->intersect(&obj);
    }
    glDisable(GL_DEPTH_TEST);
    ray->drawRay(view, projection, flatShader);
    if(picker && picker->hit_object){
        drawSelectionBox(picker->hit_object, view, projection, flatShader);
    }
    glEnable(GL_DEPTH_TEST);
    ImGuiIO& io = ImGui::GetIO();
    static bool wasLeftMousePressed = false;
    bool isLeftMousePressed = glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (isLeftMousePressed && !wasLeftMousePressed && !io.WantCaptureMouse) {
        double mouseX, mouseY;
        glfwGetCursorPos(this->window, &mouseX, &mouseY);
        picker->pick(mouseX, mouseY, display_w, display_h, view, projection, scene);
    }
    wasLeftMousePressed = isLeftMousePressed;
    handleKeyboardEvents(deltaTime);
    uiManager->render();
    glfwManager->update();
}
void Application::loadBoxScene() {
    Camera* activeCamera = cameras[activeCameraIndex];
    BasicShapesGenerator::loadDefaultBoxScene(scene, lighting, activeCamera);
}
void Application::handleKeyboardEvents(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    Camera* activeCamera = cameras[activeCameraIndex];
    if (!io.WantCaptureKeyboard) {
        float scrollOffset = io.MouseWheel;
        if (scrollOffset != 0.0f) {
            float currentFov = activeCamera->getFov();
            currentFov -= scrollOffset * 2.5f;
            currentFov = std::max(10.0f, std::min(120.0f, currentFov));
            activeCamera->setFov(currentFov);
        }
        if (glfwGetKey(this->window, GLFW_KEY_W) == GLFW_PRESS) {
            activeCamera->movement(MovementDirection::FORWARD, deltaTime);
        }
        if (glfwGetKey(this->window, GLFW_KEY_S) == GLFW_PRESS) {
            activeCamera->movement(MovementDirection::BACKWARD, deltaTime);
        }
        if (glfwGetKey(this->window, GLFW_KEY_A) == GLFW_PRESS) {
            activeCamera->movement(MovementDirection::LEFT, deltaTime);
        }
        if (glfwGetKey(this->window, GLFW_KEY_D) == GLFW_PRESS) {
            activeCamera->movement(MovementDirection::RIGHT, deltaTime);
        }
        if(glfwGetKey(this->window, GLFW_KEY_SPACE) == GLFW_PRESS){
            activeCamera->movement(MovementDirection::UP, deltaTime);
        }
        if(glfwGetKey(this->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            activeCamera->movement(MovementDirection::DOWN, deltaTime);
        }
    }
    static bool isMouseCaptured = false;
    static double lastMouseX = 0.0;
    static double lastMouseY = 0.0;
    if (glfwGetKey(this->window, GLFW_KEY_Z) == GLFW_PRESS || (glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)) {
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
            activeCamera->rotate(xOffset, yOffset);
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
    delete scene;
    scene = nullptr;
    delete lighting;
    lighting = nullptr;
    delete shader;
    shader = nullptr;
    delete flatShader;
    flatShader = nullptr;
    delete shadowDepthShader;
    shadowDepthShader = nullptr;
    delete renderer;
    renderer = nullptr;
    delete glfwManager;
    glfwManager = nullptr;
    delete ray;
    ray = nullptr;
    delete picker;
    picker = nullptr;
    for (Camera* c : cameras) {
        delete c;
    }
    cameras.clear();
}
