#include "../../include/tools/UIManager.h"
#include "../../include/Application.h"
#include "../../include/Scene.h"
#include "../../include/Mesh.h"
#include "../../include/Lighting.h"
#include "../../include/tools/GLTFManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../../include/Ray.h"
#include "../../include/tools/InputPicker.h"
#include "../../include/Camera.h"
#include "../../include/tools/BasicShapesGenerator.h"
#include "../../include/tools/ShadowManager.h"
#include "../../include/tools/tinyfiledialogs.h"
#include <cstring>
using namespace std;
void UIManager::addInstructionsUI(){
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Instrucciones");
    ImGui::Separator();
    ImGui::Text("Mover camara: WASD");
    ImGui::Text("Space para ascender, Shift para descender");
    ImGui::Text("Rotar camara: Mantener presionado Z o click derecho y mover mouse");
    ImGui::Text("Zoom: Rueda del mouse");
    ImGui::Text("Acercar/Alejar: Rueda del mouse");
    ImGui::Text("Ray Picking: Clic izquierdo en un objeto");
    ImGui::Text("Generar solidos: Clic en el objeto y luego en generar");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}
void UIManager::generateRevolutionSolid(Scene* scene, InputPicker* picker, ShapeType& activeShapeType){
    vector<Vertex> vertices = RevolutionSolidGenerator::generate(currentSegments, radialSegments, samplePointsPerSegment);
    if (!vertices.empty()) {
        Mesh* newMesh = new Mesh(vertices);
        SceneObject newObj;
        std::string namePrefix = "Solido";
        if (activeShapeType == ShapeType::SPHERE) namePrefix = "Esfera";
        else if (activeShapeType == ShapeType::CYLINDER) namePrefix = "Cilindro";
        else if (activeShapeType == ShapeType::CONE) namePrefix = "Cono";
        newObj.name = namePrefix + " " + to_string(scene->objects.size());
        newObj.type = MeshType::REVOLUTION_SOLID;
        newObj.shape = activeShapeType;           
        newObj.meshPointer = newMesh;
        newObj.position = glm::vec3(0.0f, -4.0f, 0.0f);
        newObj.rotation = glm::vec3(0.0f);
        newObj.scale = glm::vec3(1.0f);
        scene->addObject(newObj);
        selectedObjectIndex = (int)scene->objects.size() - 1;
    }
}
void UIManager::addObjectGenerationUI(Scene* scene, InputPicker* picker, ShapeType& activeShapeType){
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Generador de Solidos de Revolucion");
    ImGui::Separator();
    ImGui::Text("Presets de Contorno:");
    if (ImGui::Button("Cilindro", ImVec2(70, 0))) { RevolutionSolidGenerator::loadCylinderPreset(currentSegments); activeShapeType = ShapeType::CYLINDER; generateRevolutionSolid(scene, picker, activeShapeType); }
    ImGui::SameLine();
    if (ImGui::Button("Cono", ImVec2(70, 0))) { RevolutionSolidGenerator::loadConePreset(currentSegments); activeShapeType = ShapeType::CONE; generateRevolutionSolid(scene, picker, activeShapeType); }
    ImGui::SameLine();
    if (ImGui::Button("Esfera", ImVec2(70, 0))) { RevolutionSolidGenerator::loadSpherePreset(currentSegments); activeShapeType = ShapeType::SPHERE; generateRevolutionSolid(scene, picker, activeShapeType); 
    }
    ImGui::SameLine();
    if (ImGui::Button("Pirámide", ImVec2(70, 0))) {
        std::vector<Vertex> vertices = BasicShapesGenerator::generatePyramid(1.0f, 1.0f);
        Mesh* newMesh = new Mesh(vertices);
        SceneObject newObj;
        newObj.name = "Piramide " + std::to_string(scene->objects.size());
        newObj.type = MeshType::REVOLUTION_SOLID;
        newObj.shape = ShapeType::PYRAMID;
        newObj.meshPointer = newMesh;
        newObj.position = glm::vec3(0.0f, -4.0f, 0.0f);
        newObj.rotation = glm::vec3(0.0f);
        newObj.scale = glm::vec3(2.0f);
        newObj.color = glm::vec3(0.0f, 1.0f, 1.0f);
        newObj.reflectivity = 0.8f; 
        newObj.metallicValue = 0.8f;
        newObj.roughnessValue = 0.0f;
        newObj.aoValue = 0.2f;
        scene->addObject(newObj);
        selectedObjectIndex = (int)scene->objects.size() - 1;
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}
void UIManager::addPickerUI(Scene* scene, InputPicker* picker){
    if(picker->hit_object){
        SceneObject* selectedObject = picker->hit_object;
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Objeto seleccionado: %s", selectedObject->name.c_str());
        ImGui::Separator();
        ImGui::DragFloat3("Posicion del objeto", glm::value_ptr(selectedObject->position), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::SliderFloat3("Rotacion del objeto", glm::value_ptr(selectedObject->rotation), -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat3("Escala del objeto", glm::value_ptr(selectedObject->scale), 0.02f, 0.01f, 10.0f, "%.2f");
        ImGui::ColorEdit3("Color del objeto", glm::value_ptr(selectedObject->color));
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Propiedades PBR");
        ImGui::SliderFloat("Metallic", &selectedObject->metallicValue, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &selectedObject->roughnessValue, 0.0f, 1.0f);
        ImGui::SliderFloat("Ambient Occlusion", &selectedObject->aoValue, 0.0f, 1.0f);
        if (ImGui::Button("Eliminar Objeto", ImVec2(-FLT_MIN, 0))) {
            scene->removeObject(selectedObjectIndex);
            selectedObjectIndex = -1;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }
}
void UIManager::addIlluminationUI(Lighting* lighting){
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Configuracion de Iluminacion");
    ImGui::Separator();
    const char* shadingModes[] = { "Flat Shading", "Lambert (Difusa)", "Phong", "Blinn-Phong" };
    int currentMode = static_cast<int>(lighting->activeMode);
    if (ImGui::Combo("Modelo", &currentMode, shadingModes, IM_ARRAYSIZE(shadingModes))) {
        lighting->activeMode = static_cast<ShadingMode>(currentMode);
    }
    if (!lighting->lights.empty()) {
        Light& mainLight = lighting->lights[0];
        ImGui::DragFloat3("Posicion Luz", glm::value_ptr(mainLight.position), 0.1f, -25.0f, 25.0f, "%.1f");
        ImGui::ColorEdit3("Color Luz", glm::value_ptr(mainLight.color));
        ImGui::SliderFloat("Intensidad Luz", &mainLight.intensity, 0.0f, 5.0f, "%.2f");
        ImGui::SliderFloat("Intensidad Ambiental", &mainLight.ambientIntensity, 0.0f, 1.0f, "%.2f");
    }
    ImGui::Separator();
    ImGui::SliderFloat("Exposicion PBR (Brillo)", &lighting->exposure, 0.1f, 10.0f, "%.2f");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}
void UIManager::addRaycastUI(Ray* ray){
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Ray Casting");
    ImGui::Separator();
    if(ray){
        ImGui::DragFloat3("Origin", glm::value_ptr(ray->origin), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::DragFloat3("Direction", glm::value_ptr(ray->direction), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::ColorEdit4("Color del rayo", &ray->rgba.r);
        ImGui::DragFloat("t_min", &ray->t_min, 0.05f, 0.01f, 15.0f, "%.2f");
        ImGui::DragFloat("t_max", &ray->t_max, 0.05f, 0.01f, 100.0f, "%.2f");
        if (ray->hit_object) {
            std::string name = ray->hit_object->name;
            ray->hit_object->color = glm::vec3(ray->rgba.r, ray->rgba.g, ray->rgba.b);
        }
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}
void UIManager::addFileManagementUI(Application* app, Scene* scene){
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Gestion de Archivos de Escena");
    ImGui::Separator();
    if (ImGui::Button("Importar Modelo GLTF/GLB", ImVec2(-FLT_MIN, 0))) {
        const char* filterPatterns[3] = { "*.gltf", "*.glb", "*.gdb" };
        const char* path = tinyfd_openFileDialog("Importar Modelo", "", 3, filterPatterns, "Modelos 3D (*.gltf, *.glb, *.gdb)", 0);
        if (path) {
            strncpy(importPathBuffer, path, IM_ARRAYSIZE(importPathBuffer) - 1);
            importPathBuffer[IM_ARRAYSIZE(importPathBuffer) - 1] = '\0';
            GLTFManager* gltf = new GLTFManager();
            if (gltf->loadModel(importPathBuffer)) {
                gltf->setupGL();
                SceneObject newObj;
                std::string pathStr(importPathBuffer);
                size_t lastSlash = pathStr.find_last_of("/\\");
                std::string fileName = (lastSlash == std::string::npos) ? pathStr : pathStr.substr(lastSlash + 1);
                newObj.name = fileName;
                newObj.gltfPath = pathStr;
                newObj.type = MeshType::GLTF;
                newObj.meshPointer = gltf;

                glm::vec3 minAABB(FLT_MAX);
                glm::vec3 maxAABB(-FLT_MAX);
                const auto& vertices = gltf->getVertices();
                if (!vertices.empty()) {
                    for (const auto& v : vertices) {
                        minAABB = glm::min(minAABB, v.position);
                        maxAABB = glm::max(maxAABB, v.position);
                    }
                    glm::vec3 center = (minAABB + maxAABB) * 0.5f;
                    glm::vec3 size = maxAABB - minAABB;
                    float maxAxis = std::max(size.x, std::max(size.y, size.z));
                    float scaleFactor = (maxAxis > 0.0f) ? (2.0f / maxAxis) : 1.0f;
                    
                    newObj.scale = glm::vec3(scaleFactor);
                    newObj.pivotOffset = -center;
                    newObj.position = glm::vec3(0.0f);
                } else {
                    newObj.position = glm::vec3(0.0f);
                    newObj.scale = glm::vec3(1.0f);
                    newObj.pivotOffset = glm::vec3(0.0f);
                }
                if (fileName == "jarron.glb") {
                    newObj.position = glm::vec3(-2.15f, -7.25f, 0.00f);
                    newObj.rotation = glm::vec3(-91.1f, 0.0f, 0.0f);
                    newObj.scale = glm::vec3(0.06f, 0.06f, 0.06f);
                    newObj.color = glm::vec3(1.0f, 1.0f, 1.0f); 
                    newObj.metallicValue = 0.0f;
                    newObj.roughnessValue = 0.5f;
                    newObj.aoValue = 1.0f;
                } else {
                    newObj.rotation = glm::vec3(0.0f);
                    newObj.color = glm::vec3(1.0f, 1.0f, 1.0f); 
                    newObj.metallicValue = 0.0f;
                    newObj.roughnessValue = 0.5f;
                    newObj.aoValue = 1.0f;
                }
                scene->addObject(newObj);
                selectedObjectIndex = (int)scene->objects.size() - 1;
                std::cout << "Modelo importado con exito: " << importPathBuffer << std::endl;
            } else {
                delete gltf;
                std::cerr << "Error al importar el modelo: " << importPathBuffer << std::endl;
            }
        }
    }
    ImGui::Spacing();
    if (ImGui::Button("Guardar Escena (.scene)", ImVec2(-FLT_MIN, 0))) {
        const char* filterPatterns[1] = { "*.scene" };
        const char* path = tinyfd_saveFileDialog("Guardar Escena", "", 1, filterPatterns, "Archivos de Escena");
        if (path) {
            strncpy(savePathBuffer, path, IM_ARRAYSIZE(savePathBuffer) - 1);
            savePathBuffer[IM_ARRAYSIZE(savePathBuffer) - 1] = '\0';
            if (app->saveProject(savePathBuffer)) {
                std::cout << "Escena guardada exitosamente en: " << savePathBuffer << std::endl;
            } else {
                std::cerr << "Error al guardar la escena en: " << savePathBuffer << std::endl;
            }
        }
    }
    ImGui::Spacing();
    if (ImGui::Button("Cargar Escena (.scene)", ImVec2(-FLT_MIN, 0))) {
        const char* filterPatterns[1] = { "*.scene" };
        const char* path = tinyfd_openFileDialog("Cargar Escena", "", 1, filterPatterns, "Archivos de Escena", 0);
        if (path) {
            strncpy(loadPathBuffer, path, IM_ARRAYSIZE(loadPathBuffer) - 1);
            loadPathBuffer[IM_ARRAYSIZE(loadPathBuffer) - 1] = '\0';
            if (app->loadProject(loadPathBuffer)) {
                selectedObjectIndex = -1;
                std::cout << "Escena cargada exitosamente desde: " << loadPathBuffer << std::endl;
            } else {
                std::cerr << "Error al cargar la escena desde: " << loadPathBuffer << std::endl;
            }
        }
    }
}
void UIManager::addCameraUI(std::vector<Camera*>& cameras, int& activeCameraIndex) {
    if (cameras.empty()) return;
    static glm::vec3 nextCameraPos(0.0f, 5.0f, 8.0f); 
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Nueva Camara de Seguridad");
    ImGui::DragFloat3("Posicion Inicial", glm::value_ptr(nextCameraPos), 0.1f, -20.0f, 20.0f, "%.2f");
    if(ImGui::Button("Agregar Camara", ImVec2(-FLT_MIN, 0))){
        Camera* newCamera = new Camera(nextCameraPos);
        cameras.push_back(newCamera); 
        activeCameraIndex = (int)cameras.size() - 1; 
    }
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Selector de Camara");
    ImGui::Separator();
    for (int i = 0; i < cameras.size(); i++) {
        std::string label = "Camara " + std::to_string(i + 1);
        ImGui::RadioButton(label.c_str(), &activeCameraIndex, i);
        if (i < cameras.size() - 1 && (i + 1) % 3 != 0) { 
            ImGui::SameLine(); 
        }
    }
    ImGui::Spacing();
    Camera* currentCamera = cameras[activeCameraIndex];
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Modo de Renderizado");
    ImGui::Separator();
    int currentMode = static_cast<int>(currentCamera->getRenderMode());
    if (ImGui::RadioButton("Rasterizacion", &currentMode, 0)) {
        currentCamera->setRenderMode(RenderMode::RASTERIZATION);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Ray Tracing", &currentMode, 1)) {
        for (Camera* cam : cameras) {
            cam->setRenderMode(RenderMode::RAY_TRACING);
        }
    }
    if (currentMode == 1) {
        int bounces = cameras[activeCameraIndex]->getRayBounces();
        if (ImGui::SliderInt("Rebotes Maximos", &bounces, 1, 5)) {
            for (Camera* cam : cameras) {
                cam->setRayBounces(bounces);
            }
        }
    } ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Propiedades de Transformacion");
    ImGui::Separator();
    glm::vec3 pos = currentCamera->getPosition();
    if (ImGui::DragFloat3("Posicion Camara", glm::value_ptr(pos), 0.05f, -20.0f, 20.0f, "%.2f")) {
        currentCamera->setPosition(pos);
    }
    glm::vec3 tgt = currentCamera->getTarget();
    if (ImGui::DragFloat3("Objetivo (Target)", glm::value_ptr(tgt), 0.05f, -20.0f, 20.0f, "%.2f")) {
        currentCamera->setTarget(tgt);
    }
    float fov = currentCamera->getFov();
    if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f, "%.1f")) {
        currentCamera->setFov(fov);
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}
void UIManager::addShadowModesUI(){
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Controles de sombras");
    const char* shadowModes[] = { "Sin sombras", "Sombras Planares", "Shadow Mapping (FBO)", "Shadow Volumes" };
    int currentShadowMode = static_cast<int>(ShadowManager::mode);
    if (ImGui::Combo("Modo de Sombras", &currentShadowMode, shadowModes, IM_ARRAYSIZE(shadowModes))) {
        ShadowManager::mode = static_cast<ShadowMode>(currentShadowMode);
    }
    if (ShadowManager::mode == ShadowMode::SHADOW_VOLUMES) {
        ImGui::Separator();
        ImGui::Checkbox("Ver Mallado de Volumenes de Sombras (Wireframe)", &ShadowManager::showShadowVolumes);
    }
    if (ShadowManager::mode == ShadowMode::SHADOW_MAPPING) {
        ImGui::Separator();
        const char* shadowMappingTypes[] = { "Direccional", "Spot" };
        int currentShadowMappingType = static_cast<int>(ShadowManager::shadowMappingType);
        if (ImGui::Combo("Tipo de Shadow Mapping", &currentShadowMappingType, shadowMappingTypes, IM_ARRAYSIZE(shadowMappingTypes))) {
            ShadowManager::shadowMappingType = static_cast<ShadowMappingType>(currentShadowMappingType);
        }
        ImGui::SliderFloat("Shadow Bias", &ShadowManager::biasForShadowMapping, 0.0001f, 0.05f, "%.4f");
        ImGui::Checkbox("Ver Solo Sombras (B&W)", &ShadowManager::showOnlyShadows);
        ImGui::Checkbox("Ver Mapa de Profundidad (FBO)", &ShadowManager::showDepthMap);
        ImGui::Checkbox("Usar bias adaptativo para solucionar Peter Panning", &ShadowManager::useAdaptativeBias);
        ImGui::Separator();
        ImGui::Checkbox("Activar Suavizado de Bordes (PCF)", &ShadowManager::usePCF);
        if (ShadowManager::usePCF) {
            ImGui::SliderInt("Radio de Kernel (PCF)", &ShadowManager::pcfKernelRadius, 1, 3, "%d px"); 
            ImGui::Text("Tamaño del Filtro: %dx%d (%d muestras)", 
                        ShadowManager::pcfKernelRadius * 2 + 1, 
                        ShadowManager::pcfKernelRadius * 2 + 1,
                        (ShadowManager::pcfKernelRadius * 2 + 1) * (ShadowManager::pcfKernelRadius * 2 + 1));
        }
    }
}
UIManager::UIManager(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}
UIManager::~UIManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
void UIManager::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void UIManager::drawInspector(Application* app, Scene* scene, Lighting* lighting, Ray* ray, InputPicker* picker, std::vector<Camera*>& cameras, int& activeCameraIndex) {
    if (!scene || !lighting) return;
    static ShapeType activeShapeType = ShapeType::CYLINDER;
    float targetWidth = 500.0f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(targetWidth, viewport->WorkSize.y), ImGuiCond_Always);
    ImGui::Begin("Controles del Escenario", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    addInstructionsUI();
    addObjectGenerationUI(scene, picker, activeShapeType);
    addCameraUI(cameras, activeCameraIndex);
    addPickerUI(scene, picker);
    addIlluminationUI(lighting);
    addRaycastUI(ray);
    addShadowModesUI();
    addFileManagementUI(app, scene);
    if (cameras[activeCameraIndex]) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "Escena Especial");
        ImGui::Separator();
        if (ImGui::Button("Cargar Escena de la Caja", ImVec2(-FLT_MIN, 40))) {
            BasicShapesGenerator::loadDefaultBoxScene(scene, lighting, cameras[activeCameraIndex]);
        }
    }
    ImGui::End();
}
void UIManager::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}