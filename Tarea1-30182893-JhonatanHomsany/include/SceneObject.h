#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tools/TextureManager.h"

enum class MeshType {
    GLTF,
    REVOLUTION_SOLID
};
enum class ShapeType{
    NONE,
    SPHERE,
    CONE,
    CYLINDER,
    PLANE,
    CUBE,
    PYRAMID
};
struct SceneObject {
    std::string name;
    std::string gltfPath = "";
    MeshType type;
    void* meshPointer;
    
    std::string albedoPath = "";
    std::string normalPath = "";
    std::string bumpPath = "";
    std::string metallicPath = "";
    std::string roughnessPath = "";
    std::string aoPath = "";
    ShapeType shape = ShapeType::NONE;
    float localRadius = 1.0f;
    float reflectivity = 0.0f;
    float transparency = 0.0f;
    float refractiveIndex = 1.0f;
    unsigned int normalMapID = 0;
    unsigned int bumpMapID = 0;
    unsigned int albedoMapID = 0;
    unsigned int metallicMapID = 0;
    unsigned int roughnessMapID = 0;
    unsigned int aoMapID = 0;
    float metallicValue = 0.0f;
    float roughnessValue = 0.5f;
    float aoValue = 1.0f;
    TextureType textureType = TextureType::NONE;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); 
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 pivotOffset = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(0.8f);
    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        model = glm::translate(model, pivotOffset);
        return model;
    }
};
