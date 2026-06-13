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
#include <fstream>
#include <sstream>
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
    
    glGenBuffers(1, &ssboTriangles);
    
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
    uiManager->drawInspector(this, scene, lighting, ray, picker, cameras, activeCameraIndex);
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
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glEnable(GL_DEPTH_TEST);
        scene->draw(flatShader, renderer, view, projection, nullptr, activeCamera->getPosition());
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

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
        glUniform1f(glGetUniformLocation(raytraceShader->ID, "exposure"), lighting->exposure);
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "shadingMode"), static_cast<int>(lighting->activeMode));
        if (!lighting->lights.empty()) {
            glUniform1f(glGetUniformLocation(raytraceShader->ID, "shininess"), lighting->lights[0].shininess);
            glUniform1f(glGetUniformLocation(raytraceShader->ID, "specularStrength"), lighting->lights[0].specularStrength);
        } else {
            glUniform1f(glGetUniformLocation(raytraceShader->ID, "shininess"), 32.0f);
            glUniform1f(glGetUniformLocation(raytraceShader->ID, "specularStrength"), 0.5f);
        }
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
        
        std::vector<GLuint> activeTextures;
        auto getMappedTexID = [&](GLuint tex) -> int {
            if (tex == 0) return 0;
            auto it = std::find(activeTextures.begin(), activeTextures.end(), tex);
            if (it != activeTextures.end()) {
                return std::distance(activeTextures.begin(), it) + 1;
            }
            if (activeTextures.size() < 5) {
                activeTextures.push_back(tex);
                return activeTextures.size();
            }
            return 0;
        };
        
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
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".metallic").c_str()), obj.metallicValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".roughness").c_str()), obj.roughnessValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".ao").c_str()), obj.aoValue);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".textureType").c_str()), static_cast<int>(obj.textureType));
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".hasBumpMap").c_str()), obj.bumpMapID != 0 ? 1 : 0);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".albedoMapID").c_str()), getMappedTexID(obj.albedoMapID));
                if (obj.bumpMapID != 0) {
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, obj.bumpMapID);
                    glUniform1i(glGetUniformLocation(raytraceShader->ID, "globalBumpMap"), 3);
                }

                sphereCount++;
            }
        }
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numSpheres"), sphereCount);
        int planeCount = 0;
        for (SceneObject& obj : scene->objects) {
            if (obj.shape == ShapeType::PLANE && planeCount < 10) {
                std::string base = "planes[" + std::to_string(planeCount) + "]";
                glm::mat4 invModel = glm::inverse(obj.getModelMatrix());
                glUniformMatrix4fv(glGetUniformLocation(raytraceShader->ID, (base + ".invModel").c_str()), 1, GL_FALSE, glm::value_ptr(invModel));
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(obj.color));
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".reflectivity").c_str()), obj.reflectivity);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".transparency").c_str()), obj.transparency);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".refractiveIndex").c_str()), obj.refractiveIndex);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".metallic").c_str()), obj.metallicValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".roughness").c_str()), obj.roughnessValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".ao").c_str()), obj.aoValue);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".textureType").c_str()), static_cast<int>(obj.textureType));
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".hasBumpMap").c_str()), obj.bumpMapID != 0 ? 1 : 0);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".albedoMapID").c_str()), getMappedTexID(obj.albedoMapID));
                planeCount++;
            }
        }
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numPlanes"), planeCount);
        int triCount = 0;
        int cylCount = 0;
        int boxCount = 0;
        std::vector<SSBOTriangle> trianglesData;
        int numMeshGroups = 0;
        for (SceneObject& obj : scene->objects) {
            if (obj.shape == ShapeType::CYLINDER && cylCount < 10) {
                std::string base = "cylinders[" + std::to_string(cylCount) + "]";
                glm::mat4 invModel = glm::inverse(obj.getModelMatrix());
                glUniformMatrix4fv(glGetUniformLocation(raytraceShader->ID, (base + ".invModel").c_str()), 1, GL_FALSE, glm::value_ptr(invModel));
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(obj.color));
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".reflectivity").c_str()), obj.reflectivity);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".transparency").c_str()), obj.transparency);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".refractiveIndex").c_str()), obj.refractiveIndex);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".metallic").c_str()), obj.metallicValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".roughness").c_str()), obj.roughnessValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".ao").c_str()), obj.aoValue);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".textureType").c_str()), static_cast<int>(obj.textureType));
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".hasBumpMap").c_str()), obj.bumpMapID != 0 ? 1 : 0);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".albedoMapID").c_str()), getMappedTexID(obj.albedoMapID));
                cylCount++;
            } else if (obj.shape == ShapeType::CUBE && boxCount < 10) {
                std::string base = "boxes[" + std::to_string(boxCount) + "]";
                glm::mat4 invModel = glm::inverse(obj.getModelMatrix());
                glUniformMatrix4fv(glGetUniformLocation(raytraceShader->ID, (base + ".invModel").c_str()), 1, GL_FALSE, glm::value_ptr(invModel));
                glUniform3fv(glGetUniformLocation(raytraceShader->ID, (base + ".color").c_str()), 1, glm::value_ptr(obj.color));
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".reflectivity").c_str()), obj.reflectivity);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".transparency").c_str()), obj.transparency);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".refractiveIndex").c_str()), obj.refractiveIndex);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".metallic").c_str()), obj.metallicValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".roughness").c_str()), obj.roughnessValue);
                glUniform1f(glGetUniformLocation(raytraceShader->ID, (base + ".ao").c_str()), obj.aoValue);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".textureType").c_str()), static_cast<int>(obj.textureType));
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".hasBumpMap").c_str()), obj.bumpMapID != 0 ? 1 : 0);
                glUniform1i(glGetUniformLocation(raytraceShader->ID, (base + ".albedoMapID").c_str()), getMappedTexID(obj.albedoMapID));
                boxCount++;
            } else if (obj.shape == ShapeType::NONE && obj.type == MeshType::GLTF && obj.meshPointer != nullptr) {
                const auto& vertices = static_cast<GLTFManager*>(obj.meshPointer)->getVertices();
                if (!vertices.empty() && numMeshGroups < 10) {
                    glm::vec3 minAABB(FLT_MAX);
                    glm::vec3 maxAABB(-FLT_MAX);
                    int startIndex = triCount;
                    
                    glm::mat4 model = obj.getModelMatrix();
                    for (size_t i = 0; i + 2 < vertices.size() && triCount < 20000; i += 3) {
                        glm::vec3 v0 = glm::vec3(model * glm::vec4(vertices[i].position, 1.0f));
                        glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[i+1].position, 1.0f));
                        glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[i+2].position, 1.0f));
                        
                        minAABB = glm::min(minAABB, glm::min(v0, glm::min(v1, v2)));
                        maxAABB = glm::max(maxAABB, glm::max(v0, glm::max(v1, v2)));
                        
                        glm::vec3 edge1 = v1 - v0;
                        glm::vec3 edge2 = v2 - v0;
                        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
                        SSBOTriangle tri;
                        tri.v0 = glm::vec4(v0, 0.0f);
                        tri.v1 = glm::vec4(v1, 0.0f);
                        tri.v2 = glm::vec4(v2, 0.0f);
                        tri.normal = glm::vec4(normal, 0.0f);
                        tri.color = glm::vec4(obj.color, 0.0f);
                        tri.reflectivity = obj.reflectivity;
                        tri.transparency = obj.transparency;
                        tri.refractiveIndex = obj.refractiveIndex;
                        tri.metallic = obj.metallicValue;
                        tri.roughness = obj.roughnessValue;
                        tri.ao = obj.aoValue;
                        GLuint actualTex = obj.albedoMapID;
                        int isTextured = static_cast<int>(obj.textureType);
                        GLTFManager* gltfMgr = static_cast<GLTFManager*>(obj.meshPointer);
                        if (!gltfMgr->materials.empty() && gltfMgr->materials[0].baseColorTexture != 0) {
                            actualTex = gltfMgr->materials[0].baseColorTexture;
                            isTextured = 5;
                        }
                        tri.textureType = isTextured;
                        tri.hasBumpMap = obj.bumpMapID != 0 ? 1 : 0;
                        tri.albedoMapID = getMappedTexID(actualTex);
                        tri.local_v0 = glm::vec4(vertices[i].position, 0.0f);
                        tri.local_v1 = glm::vec4(vertices[i+1].position, 0.0f);
                        tri.local_v2 = glm::vec4(vertices[i+2].position, 0.0f);
                        tri.uv0 = vertices[i].textureCoordinates;
                        tri.uv1 = vertices[i+1].textureCoordinates;
                        tri.uv2 = vertices[i+2].textureCoordinates;
                        tri.pad4[0] = 0.0f;
                        tri.pad4[1] = 0.0f;
                        trianglesData.push_back(tri);
                        triCount++;
                    }
                    
                    std::string groupBase = "meshGroups[" + std::to_string(numMeshGroups) + "]";
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (groupBase + ".minAABB").c_str()), 1, glm::value_ptr(minAABB));
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (groupBase + ".maxAABB").c_str()), 1, glm::value_ptr(maxAABB));
                    glUniform1i(glGetUniformLocation(raytraceShader->ID, (groupBase + ".startIndex").c_str()), startIndex);
                    glUniform1i(glGetUniformLocation(raytraceShader->ID, (groupBase + ".count").c_str()), triCount - startIndex);
                    numMeshGroups++;
                }
            } else if (obj.shape == ShapeType::NONE && obj.type == MeshType::REVOLUTION_SOLID && obj.meshPointer != nullptr) {
                glm::mat4 model = obj.getModelMatrix();
                const auto& vertices = static_cast<Mesh*>(obj.meshPointer)->getVertices();
                if (!vertices.empty() && numMeshGroups < 10) {
                    glm::vec3 minAABB(FLT_MAX);
                    glm::vec3 maxAABB(-FLT_MAX);
                    int startIndex = triCount;
                    
                    for (size_t i = 0; i + 2 < vertices.size() && triCount < 20000; i += 3) {
                        glm::vec3 v0 = glm::vec3(model * glm::vec4(vertices[i].position, 1.0f));
                        glm::vec3 v1 = glm::vec3(model * glm::vec4(vertices[i+1].position, 1.0f));
                        glm::vec3 v2 = glm::vec3(model * glm::vec4(vertices[i+2].position, 1.0f));
                        
                        minAABB = glm::min(minAABB, glm::min(v0, glm::min(v1, v2)));
                        maxAABB = glm::max(maxAABB, glm::max(v0, glm::max(v1, v2)));
                        
                        glm::vec3 edge1 = v1 - v0;
                        glm::vec3 edge2 = v2 - v0;
                        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
                        SSBOTriangle tri;
                        tri.v0 = glm::vec4(v0, 0.0f);
                        tri.v1 = glm::vec4(v1, 0.0f);
                        tri.v2 = glm::vec4(v2, 0.0f);
                        tri.normal = glm::vec4(normal, 0.0f);
                        tri.color = glm::vec4(obj.color, 0.0f);
                        tri.reflectivity = obj.reflectivity;
                        tri.transparency = obj.transparency;
                        tri.refractiveIndex = obj.refractiveIndex;
                        tri.metallic = obj.metallicValue;
                        tri.roughness = obj.roughnessValue;
                        tri.ao = obj.aoValue;
                        tri.textureType = static_cast<int>(obj.textureType);
                        tri.hasBumpMap = obj.bumpMapID != 0 ? 1 : 0;
                        tri.albedoMapID = getMappedTexID(obj.albedoMapID);
                        tri.local_v0 = glm::vec4(vertices[i].position, 0.0f);
                        tri.local_v1 = glm::vec4(vertices[i+1].position, 0.0f);
                        tri.local_v2 = glm::vec4(vertices[i+2].position, 0.0f);
                        tri.uv0 = glm::vec2(0.0f);
                        tri.uv1 = glm::vec2(0.0f);
                        tri.uv2 = glm::vec2(0.0f);
                        tri.pad4[0] = 0.0f;
                        tri.pad4[1] = 0.0f;
                        trianglesData.push_back(tri);
                        triCount++;
                    }
                    
                    std::string groupBase = "meshGroups[" + std::to_string(numMeshGroups) + "]";
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (groupBase + ".minAABB").c_str()), 1, glm::value_ptr(minAABB));
                    glUniform3fv(glGetUniformLocation(raytraceShader->ID, (groupBase + ".maxAABB").c_str()), 1, glm::value_ptr(maxAABB));
                    glUniform1i(glGetUniformLocation(raytraceShader->ID, (groupBase + ".startIndex").c_str()), startIndex);
                    glUniform1i(glGetUniformLocation(raytraceShader->ID, (groupBase + ".count").c_str()), triCount - startIndex);
                    numMeshGroups++;
                }
            }
        }
        
        if (!trianglesData.empty()) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboTriangles);
            glBufferData(GL_SHADER_STORAGE_BUFFER, trianglesData.size() * sizeof(SSBOTriangle), trianglesData.data(), GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboTriangles);
        }
        
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numTriangles"), triCount);
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numMeshGroups"), numMeshGroups);
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numCylinders"), cylCount);
        glUniform1i(glGetUniformLocation(raytraceShader->ID, "numBoxes"), boxCount);
        
        for (int i = 0; i < 5; i++) {
            glActiveTexture(GL_TEXTURE3 + i + 1);
            if (i < activeTextures.size()) {
                glBindTexture(GL_TEXTURE_2D, activeTextures[i]);
            } else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            std::string name = "albedo" + std::to_string(i + 1);
            glUniform1i(glGetUniformLocation(raytraceShader->ID, name.c_str()), 3 + i + 1);
        }

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }
    ray->hit_object = nullptr;
    ray->hit_t = ray->t_max;
    for(SceneObject &obj : scene->objects){
        ray->intersect(&obj);
    }
    ray->drawRay(view, projection, flatShader);
    
    if(picker && picker->hit_object){
        glDisable(GL_DEPTH_TEST);
        drawSelectionBox(picker->hit_object, view, projection, flatShader);
        glEnable(GL_DEPTH_TEST);
    }
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

bool Application::saveProject(const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    if (lighting) {
        out << "[Lighting]\n";
        out << "activeMode=" << static_cast<int>(lighting->activeMode) << "\n";
        out << "exposure=" << lighting->exposure << "\n";
        if (!lighting->lights.empty()) {
            out << "light_position=" << lighting->lights[0].position.x << " " << lighting->lights[0].position.y << " " << lighting->lights[0].position.z << "\n";
            out << "light_color=" << lighting->lights[0].color.x << " " << lighting->lights[0].color.y << " " << lighting->lights[0].color.z << "\n";
            out << "light_intensity=" << lighting->lights[0].intensity << "\n";
            out << "light_ambient=" << lighting->lights[0].ambientIntensity << "\n";
        }
    }

    out << "[Cameras]\n";
    out << "count=" << cameras.size() << "\n";
    out << "active=" << activeCameraIndex << "\n";
    for (size_t i = 0; i < cameras.size(); ++i) {
        glm::vec3 pos = cameras[i]->getPosition();
        glm::vec3 tgt = cameras[i]->getTarget();
        glm::vec3 up = cameras[i]->getUp();
        out << "cam_" << i << "_pos=" << pos.x << " " << pos.y << " " << pos.z << "\n";
        out << "cam_" << i << "_tgt=" << tgt.x << " " << tgt.y << " " << tgt.z << "\n";
        out << "cam_" << i << "_up=" << up.x << " " << up.y << " " << up.z << "\n";
        out << "cam_" << i << "_mode=" << static_cast<int>(cameras[i]->getRenderMode()) << "\n";
    }

    if (scene) {
        out << "[Objects]\n";
        out << "count=" << scene->objects.size() << "\n";
        for (size_t i = 0; i < scene->objects.size(); ++i) {
            const auto& obj = scene->objects[i];
            out << "obj_" << i << "_name=" << obj.name << "\n";
            out << "obj_" << i << "_gltfPath=" << obj.gltfPath << "\n";
            out << "obj_" << i << "_type=" << static_cast<int>(obj.type) << "\n";
            out << "obj_" << i << "_shape=" << static_cast<int>(obj.shape) << "\n";
            out << "obj_" << i << "_pos=" << obj.position.x << " " << obj.position.y << " " << obj.position.z << "\n";
            out << "obj_" << i << "_rot=" << obj.rotation.x << " " << obj.rotation.y << " " << obj.rotation.z << "\n";
            out << "obj_" << i << "_scale=" << obj.scale.x << " " << obj.scale.y << " " << obj.scale.z << "\n";
            out << "obj_" << i << "_color=" << obj.color.x << " " << obj.color.y << " " << obj.color.z << "\n";
            out << "obj_" << i << "_metallic=" << obj.metallicValue << "\n";
            out << "obj_" << i << "_roughness=" << obj.roughnessValue << "\n";
            out << "obj_" << i << "_ao=" << obj.aoValue << "\n";
            out << "obj_" << i << "_textureType=" << static_cast<int>(obj.textureType) << "\n";
            if(!obj.albedoPath.empty()) out << "obj_" << i << "_albedoPath=" << obj.albedoPath << "\n";
            if(!obj.normalPath.empty()) out << "obj_" << i << "_normalPath=" << obj.normalPath << "\n";
            if(!obj.bumpPath.empty()) out << "obj_" << i << "_bumpPath=" << obj.bumpPath << "\n";
            if(!obj.metallicPath.empty()) out << "obj_" << i << "_metallicPath=" << obj.metallicPath << "\n";
            if(!obj.roughnessPath.empty()) out << "obj_" << i << "_roughnessPath=" << obj.roughnessPath << "\n";
            if(!obj.aoPath.empty()) out << "obj_" << i << "_aoPath=" << obj.aoPath << "\n";
        }
    }

    out.close();
    return true;
}

bool Application::loadProject(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    if (scene) {
        while (!scene->objects.empty()) {
            scene->removeObject(scene->objects.size() - 1);
        }
    }

    for (Camera* c : cameras) delete c;
    cameras.clear();
    activeCameraIndex = 0;

    std::string line;
    std::string currentSection = "";
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line[0] == '[') {
            currentSection = line;
            continue;
        }

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);
        std::istringstream iss(val);

        if (currentSection == "[Lighting]" && lighting) {
            if (key == "activeMode") { int m; iss >> m; lighting->activeMode = static_cast<ShadingMode>(m); }
            else if (key == "exposure") { iss >> lighting->exposure; }
            else if (key == "light_position" && !lighting->lights.empty()) { iss >> lighting->lights[0].position.x >> lighting->lights[0].position.y >> lighting->lights[0].position.z; }
            else if (key == "light_color" && !lighting->lights.empty()) { iss >> lighting->lights[0].color.x >> lighting->lights[0].color.y >> lighting->lights[0].color.z; }
            else if (key == "light_intensity" && !lighting->lights.empty()) { iss >> lighting->lights[0].intensity; }
            else if (key == "light_ambient" && !lighting->lights.empty()) { iss >> lighting->lights[0].ambientIntensity; }
        }
        else if (currentSection == "[Cameras]") {
            if (key == "count") {
                int count; iss >> count;
                for (int i=0; i<count; ++i) cameras.push_back(new Camera(glm::vec3(0.0f, 5.0f, 8.0f)));
            }
            else if (key == "active") { iss >> activeCameraIndex; }
            else if (key.find("cam_") == 0) {
                int idx;
                sscanf_s(key.c_str(), "cam_%d_", &idx);
                if (idx >= 0 && idx < cameras.size()) {
                    if (key.find("_pos") != std::string::npos) { glm::vec3 pos; iss >> pos.x >> pos.y >> pos.z; cameras[idx]->setPosition(pos); }
                    else if (key.find("_tgt") != std::string::npos) { glm::vec3 tgt; iss >> tgt.x >> tgt.y >> tgt.z; cameras[idx]->setTarget(tgt); }
                    else if (key.find("_up") != std::string::npos) { glm::vec3 up; iss >> up.x >> up.y >> up.z; cameras[idx]->setUp(up); }
                    else if (key.find("_mode") != std::string::npos) { int m; iss >> m; cameras[idx]->setRenderMode(static_cast<RenderMode>(m)); }
                }
            }
        }
        else if (currentSection == "[Objects]" && scene) {
            if (key == "count") {
                int count; iss >> count;
                scene->objects.resize(count);
                for(int i = 0; i < count; i++) scene->objects[i].meshPointer = nullptr;
            }
            else if (key.find("obj_") == 0) {
                int idx;
                sscanf_s(key.c_str(), "obj_%d_", &idx);
                if (idx >= 0 && idx < scene->objects.size()) {
                    auto& obj = scene->objects[idx];
                    if (key.find("_name") != std::string::npos) { obj.name = val; }
                    else if (key.find("_gltfPath") != std::string::npos) { obj.gltfPath = val; }
                    else if (key.find("_type") != std::string::npos) { int t; iss >> t; obj.type = static_cast<MeshType>(t); }
                    else if (key.find("_shape") != std::string::npos) { int s; iss >> s; obj.shape = static_cast<ShapeType>(s); }
                    else if (key.find("_pos") != std::string::npos) { iss >> obj.position.x >> obj.position.y >> obj.position.z; }
                    else if (key.find("_rot") != std::string::npos) { iss >> obj.rotation.x >> obj.rotation.y >> obj.rotation.z; }
                    else if (key.find("_scale") != std::string::npos) { iss >> obj.scale.x >> obj.scale.y >> obj.scale.z; }
                    else if (key.find("_color") != std::string::npos) { iss >> obj.color.x >> obj.color.y >> obj.color.z; }
                    else if (key.find("_metallic") != std::string::npos) { iss >> obj.metallicValue; }
                    else if (key.find("_roughness") != std::string::npos) { iss >> obj.roughnessValue; }
                    else if (key.find("_ao") != std::string::npos) { iss >> obj.aoValue; }
                    else if (key.find("_textureType") != std::string::npos) { int tt; iss >> tt; obj.textureType = static_cast<TextureType>(tt); }
                    else if (key.find("_albedoPath") != std::string::npos) { obj.albedoPath = val; obj.albedoMapID = TextureManager::loadTexture(val); }
                    else if (key.find("_normalPath") != std::string::npos) { obj.normalPath = val; obj.normalMapID = TextureManager::loadTexture(val); }
                    else if (key.find("_bumpPath") != std::string::npos) { obj.bumpPath = val; obj.bumpMapID = TextureManager::loadTexture(val); }
                    else if (key.find("_metallicPath") != std::string::npos) { obj.metallicPath = val; obj.metallicMapID = TextureManager::loadTexture(val); }
                    else if (key.find("_roughnessPath") != std::string::npos) { obj.roughnessPath = val; obj.roughnessMapID = TextureManager::loadTexture(val); }
                    else if (key.find("_aoPath") != std::string::npos) { obj.aoPath = val; obj.aoMapID = TextureManager::loadTexture(val); }
                }
            }
        }
    }
    
    if (scene) {
        for (auto& obj : scene->objects) {
            if (obj.type == MeshType::GLTF && !obj.gltfPath.empty()) {
                GLTFManager* gltf = new GLTFManager();
                if (gltf->loadModel(obj.gltfPath)) {
                    gltf->setupGL();
                    obj.meshPointer = gltf;
                    
                    glm::vec3 minAABB(FLT_MAX);
                    glm::vec3 maxAABB(-FLT_MAX);
                    const auto& vertices = gltf->getVertices();
                    if (!vertices.empty()) {
                        for (const auto& v : vertices) {
                            minAABB = glm::min(minAABB, v.position);
                            maxAABB = glm::max(maxAABB, v.position);
                        }
                        glm::vec3 center = (minAABB + maxAABB) * 0.5f;
                        obj.pivotOffset = -center;
                    }
                } else {
                    delete gltf;
                    obj.meshPointer = nullptr;
                    std::cerr << "Error al recargar modelo GLTF: " << obj.gltfPath << std::endl;
                }
            } else if (obj.type == MeshType::REVOLUTION_SOLID) {
                std::vector<Vertex> verts;
                switch (obj.shape) {
                    case ShapeType::CUBE:
                        verts = BasicShapesGenerator::generateCube(1.0f, false);
                        break;
                    case ShapeType::PLANE:
                        verts = BasicShapesGenerator::generateQuad();
                        break;
                    case ShapeType::SPHERE:
                        verts = BasicShapesGenerator::generateSphere(0.5f, 30, 30);
                        break;
                    case ShapeType::CYLINDER:
                        verts = BasicShapesGenerator::generateCylinder(0.5f, 1.0f, 15);
                        break;
                    case ShapeType::PYRAMID:
                        verts = BasicShapesGenerator::generatePyramid(1.0f, 1.0f);
                        break;
                    case ShapeType::CONE:
                        verts = BasicShapesGenerator::generatePyramid(1.0f, 1.0f);
                        break;
                    default:
                        // If shape is NONE, we might not have the original points, 
                        // fallback to a cube so it's not invisible.
                        verts = BasicShapesGenerator::generateCube(1.0f, false);
                        break;
                }
                if (!verts.empty()) {
                    obj.meshPointer = new Mesh(verts);
                }
            }
        }
    }
    
    if (cameras.empty()) {
        cameras.push_back(new Camera(glm::vec3(0.0f, 5.0f, 8.0f)));
        activeCameraIndex = 0;
    } else {
        if (activeCameraIndex >= cameras.size()) activeCameraIndex = 0;
    }

    in.close();
    return true;
}
