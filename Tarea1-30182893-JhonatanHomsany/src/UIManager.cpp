#include "../include/UIManager.h"
#include "../include/Scene.h"
#include "../include/Mesh.h"
#include "../include/Lighting.h"
#include "../include/GLTFManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../include/Ray.h"
#include "../include/InputPicker.h"

using namespace std;

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

void UIManager::drawInspector(Scene* scene, Lighting* lighting, Ray* ray, InputPicker* picker) {
    if (!scene || !lighting) return;

    static ShapeType activeShapeType = ShapeType::CYLINDER;

    float targetWidth = 320.0f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(targetWidth, viewport->WorkSize.y), ImGuiCond_Always);
    
    ImGui::Begin("Controles del Escenario", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Jerarquia de la Escena");
    ImGui::Separator();
    
    if (ImGui::BeginListBox("##Hierarchy", ImVec2(-FLT_MIN, 120))) {
        for (int i = 0; i < (int)scene->objects.size(); ++i) {
            const bool isSelected = (selectedObjectIndex == i);
            if (ImGui::Selectable((scene->objects[i].name + "##" + to_string(i)).c_str(), isSelected)) {
                selectedObjectIndex = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndListBox();
    }
    
    ImGui::Spacing();
    
    if (selectedObjectIndex >= 0 && selectedObjectIndex < (int)scene->objects.size()) {
        SceneObject& obj = scene->objects[selectedObjectIndex];
        
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Inspector: %s", obj.name.c_str());
        ImGui::Separator();
        
        ImGui::DragFloat3("Posicion", glm::value_ptr(obj.position), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::SliderFloat3("Rotacion", glm::value_ptr(obj.rotation), -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat3("Escala", glm::value_ptr(obj.scale), 0.02f, 0.01f, 10.0f, "%.2f");
        ImGui::ColorEdit3("Color", glm::value_ptr(obj.color));
        
        ImGui::Spacing();
        if (ImGui::Button("Eliminar Objeto Seleccionado", ImVec2(-FLT_MIN, 0))) {
            scene->removeObject(selectedObjectIndex);
            selectedObjectIndex = -1;
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // --- PANEL DE ILUMINACIÓN ---
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
        ImGui::SliderFloat("Intensidad Amb.", &mainLight.ambientIntensity, 0.0f, 1.0f, "%.2f");
        
        if (lighting->activeMode == ShadingMode::PHONG || lighting->activeMode == ShadingMode::BLINN_PHONG) {
            ImGui::SliderFloat("Fuerza Especular", &mainLight.specularStrength, 0.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("Brillo (Shininess)", &mainLight.shininess, 1.0f, 256.0f, "%.1f");
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Generador de Solidos de Revolucion");
    ImGui::Separator();
    
    ImGui::Text("Presets de Contorno:");
    if (ImGui::Button("Cilindro", ImVec2(70, 0))) { loadCylinderPreset(); activeShapeType = ShapeType::CYLINDER; }
    ImGui::SameLine();
    if (ImGui::Button("Cono", ImVec2(70, 0))) { loadConePreset(); activeShapeType = ShapeType::CONE; }
    ImGui::SameLine();
    if (ImGui::Button("Esfera", ImVec2(70, 0))) { loadSpherePreset(); activeShapeType = ShapeType::SPHERE; }

    
    if (ImGui::Button("Generar e Incorporar a Escena", ImVec2(-FLT_MIN, 40))) {
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
            newObj.position = glm::vec3(0.0f, 0.0f, 0.0f);
            newObj.rotation = glm::vec3(0.0f);
            newObj.scale = glm::vec3(1.0f);
            scene->addObject(newObj);
            selectedObjectIndex = (int)scene->objects.size() - 1;
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    //Raycast y raytracing
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Ray Tracing");
    ImGui::Separator();
    if(ray){
        ImGui::DragFloat3("Origin", glm::value_ptr(ray->origin), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::DragFloat3("Direction", glm::value_ptr(ray->direction), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::ColorEdit4("Color del rayo", &ray->rgba.r);
        ImGui::DragFloat("t_min", &ray->t_min, 0.05f, 0.01f, 15.0f, "%.2f");
        ImGui::DragFloat("t_max", &ray->t_max, 0.05f, 0.01f, 100.0f, "%.2f");
        if(ray->hit_object){
            ray->hit_object->color = glm::vec3(ray->rgba.r, ray->rgba.g, ray->rgba.b);
        }
    }

    //Picking básico usando ray casting
    ImGui::Separator();
    if(picker->hit_object){
        SceneObject* selectedObject = picker->hit_object;
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Objeto seleccionado: %s", selectedObject->name.c_str());
        ImGui::Separator();
        ImGui::DragFloat3("Posicion del objeto seleccionado", glm::value_ptr(selectedObject->position), 0.05f, -15.0f, 15.0f, "%.2f");
        ImGui::SliderFloat3("Rotacion del objeto seleccionado", glm::value_ptr(selectedObject->rotation), -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat3("Escala del objeto seleccionado", glm::value_ptr(selectedObject->scale), 0.02f, 0.01f, 10.0f, "%.2f");
        ImGui::ColorEdit3("Color del objeto seleccionado", glm::value_ptr(selectedObject->color));
    }
    
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Gestion de Archivos de Escena");
    ImGui::Separator();
    
    // Import GLTF/GLB model
    ImGui::InputText("Ruta Importar", importPathBuffer, IM_ARRAYSIZE(importPathBuffer));
    if (ImGui::Button("Importar Modelo GLTF/GLB", ImVec2(-FLT_MIN, 0))) {
        GLTFManager* gltf = new GLTFManager();
        if (gltf->loadModel(importPathBuffer)) {
            gltf->setupGL();
            SceneObject newObj;
            
            // Extract the filename from the path to make the name nicer
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
    
    // Save scene
    ImGui::InputText("Ruta Guardar", savePathBuffer, IM_ARRAYSIZE(savePathBuffer));
    if (ImGui::Button("Guardar Escena (.glb)", ImVec2(-FLT_MIN, 0))) {
        if (scene->saveScene(savePathBuffer)) {
            std::cout << "Escena guardada exitosamente en: " << savePathBuffer << std::endl;
        } else {
            std::cerr << "Error al guardar la escena en: " << savePathBuffer << std::endl;
        }
    }
    
    ImGui::Spacing();
    
    // Load scene
    ImGui::InputText("Ruta Cargar", loadPathBuffer, IM_ARRAYSIZE(loadPathBuffer));
    if (ImGui::Button("Cargar Escena (.glb)", ImVec2(-FLT_MIN, 0))) {
        if (scene->loadScene(loadPathBuffer)) {
            selectedObjectIndex = -1; // Reset selection since objects were rebuilt
            std::cout << "Escena cargada exitosamente desde: " << loadPathBuffer << std::endl;
        } else {
            std::cerr << "Error al cargar la escena desde: " << loadPathBuffer << std::endl;
        }
    }
    
    ImGui::End();
}

void UIManager::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::loadCylinderPreset() {
    currentSegments.clear();
    
    // Disco base
    ProfileSegment base;
    base.isBezier = false;
    base.p0 = glm::vec2(0.0f, -1.0f);
    base.p1 = glm::vec2(0.8f, -1.0f);
    currentSegments.push_back(base);

    // Pared vertical
    ProfileSegment wall;
    wall.isBezier = false;
    wall.p0 = glm::vec2(0.8f, -1.0f);
    wall.p1 = glm::vec2(0.8f, 1.0f);
    currentSegments.push_back(wall);

    // Disco superior
    ProfileSegment top;
    top.isBezier = false;
    top.p0 = glm::vec2(0.8f, 1.0f);
    top.p1 = glm::vec2(0.0f, 1.0f);
    currentSegments.push_back(top);
}

void UIManager::loadConePreset() {
    currentSegments.clear();
    
    // Disco base
    ProfileSegment base;
    base.isBezier = false;
    base.p0 = glm::vec2(0.0f, -1.0f);
    base.p1 = glm::vec2(0.8f, -1.0f);
    currentSegments.push_back(base);

    // Lateral inclinado hasta la punta
    ProfileSegment side;
    side.isBezier = false;
    side.p0 = glm::vec2(0.8f, -1.0f);
    side.p1 = glm::vec2(0.0f, 1.0f);
    currentSegments.push_back(side);
}

void UIManager::loadSpherePreset() {
    currentSegments.clear();

    const float KAPPA = 0.5522847498f;
    const float RADIUS = 1.0f;

    ProfileSegment bottomCurve;
    bottomCurve.isBezier = true;
    bottomCurve.p0 = glm::vec2(0.0f, -RADIUS);
    bottomCurve.p1 = glm::vec2(RADIUS * KAPPA, -RADIUS);
    bottomCurve.p2 = glm::vec2(RADIUS, -RADIUS * KAPPA);
    bottomCurve.p3 = glm::vec2(RADIUS, 0.0f);
    currentSegments.push_back(bottomCurve);

    ProfileSegment topCurve;
    topCurve.isBezier = true;
    topCurve.p0 = glm::vec2(RADIUS, 0.0f);
    topCurve.p1 = glm::vec2(RADIUS, RADIUS * KAPPA);
    topCurve.p2 = glm::vec2(RADIUS * KAPPA, RADIUS);
    topCurve.p3 = glm::vec2(0.0f, RADIUS);
    currentSegments.push_back(topCurve);
}