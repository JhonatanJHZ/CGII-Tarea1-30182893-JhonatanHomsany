#include "../../include/tools/UIManager.h"
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


using namespace std;

void UIManager::addInstructionsUI(){
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Instrucciones");
    ImGui::Separator();
    ImGui::Text("Mover camara: WASD");
    ImGui::Text("Rotar camara: Mantener presionado z y mover mouse");
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
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void UIManager::addRaycastUI(Ray* ray){
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Ray Tracing");
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

void UIManager::addFileManagementUI(Scene* scene){
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Gestion de Archivos de Escena");
    ImGui::Separator();
    
    ImGui::InputText("Ruta Importar", importPathBuffer, IM_ARRAYSIZE(importPathBuffer));
    if (ImGui::Button("Importar Modelo GLTF/GLB", ImVec2(-FLT_MIN, 0))) {
        GLTFManager* gltf = new GLTFManager();
        if (gltf->loadModel(importPathBuffer)) {
            gltf->setupGL();
            SceneObject newObj;

            std::string pathStr(importPathBuffer);
            size_t lastSlash = pathStr.find_last_of("/\\");
            std::string fileName = (lastSlash == std::string::npos) ? pathStr : pathStr.substr(lastSlash + 1);
            
            newObj.name = fileName;
            newObj.type = MeshType::GLTF;
            newObj.meshPointer = gltf;
            newObj.position = glm::vec3(0.0f);
            newObj.rotation = glm::vec3(0.0f);
            newObj.scale = glm::vec3(1.0f);
            newObj.color = glm::vec3(0.8f);
            scene->addObject(newObj);
            selectedObjectIndex = (int)scene->objects.size() - 1;
            std::cout << "Modelo importado con exito: " << importPathBuffer << std::endl;
        } else {
            delete gltf;
            std::cerr << "Error al importar el modelo: " << importPathBuffer << std::endl;
        }
    }
    
    ImGui::Spacing();
    
    ImGui::InputText("Ruta Guardar", savePathBuffer, IM_ARRAYSIZE(savePathBuffer));
    if (ImGui::Button("Guardar Escena (.glb)", ImVec2(-FLT_MIN, 0))) {
        if (scene->saveScene(savePathBuffer)) {
            std::cout << "Escena guardada exitosamente en: " << savePathBuffer << std::endl;
        } else {
            std::cerr << "Error al guardar la escena en: " << savePathBuffer << std::endl;
        }
    }
    
    ImGui::Spacing();
    
    ImGui::InputText("Ruta Cargar", loadPathBuffer, IM_ARRAYSIZE(loadPathBuffer));
    if (ImGui::Button("Cargar Escena (.glb)", ImVec2(-FLT_MIN, 0))) {
        if (scene->loadScene(loadPathBuffer)) {
            selectedObjectIndex = -1;
            std::cout << "Escena cargada exitosamente desde: " << loadPathBuffer << std::endl;
        } else {
            std::cerr << "Error al cargar la escena desde: " << loadPathBuffer << std::endl;
        }
    }
}

void UIManager::addCameraUI(Camera* camera) {
    if (!camera) return;
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Controles de Camara");
    ImGui::Separator();
    
    glm::vec3 pos = camera->getPosition();
    if (ImGui::DragFloat3("Posicion Camara", glm::value_ptr(pos), 0.05f, -20.0f, 20.0f, "%.2f")) {
        camera->setPosition(pos);
    }
    
    glm::vec3 tgt = camera->getTarget();
    if (ImGui::DragFloat3("Objetivo (Target)", glm::value_ptr(tgt), 0.05f, -20.0f, 20.0f, "%.2f")) {
        camera->setTarget(tgt);
    }
    
    float fov = camera->getFov();
    if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f, "%.1f")) {
        camera->setFov(fov);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void UIManager::addShadowModesUI(){
    const char* shadowModes[] = { "Sin sombras", "Sombras Planares", "Shadow Mapping (FBO)" };
    int currentShadowMode = static_cast<int>(ShadowManager::mode);
    if (ImGui::Combo("Modo de Sombras", &currentShadowMode, shadowModes, IM_ARRAYSIZE(shadowModes))) {
        ShadowManager::mode = static_cast<ShadowMode>(currentShadowMode);
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

void UIManager::drawInspector(Scene* scene, Lighting* lighting, Ray* ray, InputPicker* picker, Camera* camera) {
    if (!scene || !lighting) return;

    static ShapeType activeShapeType = ShapeType::CYLINDER;

    float targetWidth = 500.0f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(targetWidth, viewport->WorkSize.y), ImGuiCond_Always);
    
    ImGui::Begin("Controles del Escenario", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    addInstructionsUI();
    addObjectGenerationUI(scene, picker, activeShapeType);
    addPickerUI(scene, picker);
    addIlluminationUI(lighting);
    addRaycastUI(ray);
    addShadowModesUI();
    addFileManagementUI(scene);

    if (camera) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "Escena Especial");
        ImGui::Separator();
        if (ImGui::Button("Cargar Escena de la Caja", ImVec2(-FLT_MIN, 40))) {
            BasicShapesGenerator::loadDefaultBoxScene(scene, lighting, camera);
        }
    }
    
    ImGui::End();
}

void UIManager::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}