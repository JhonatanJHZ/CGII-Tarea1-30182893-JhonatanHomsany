#pragma once
#include <vector>
#include "SceneObject.h"
class Shader;
class Renderer;
class Lighting;
class Scene {
public:
    std::vector<SceneObject> objects;
    Scene();
    ~Scene();
    void addObject(const SceneObject& obj);
    void removeObject(size_t index);
    void draw(const Shader* shader, const Renderer* renderer, 
              const glm::mat4& view, const glm::mat4& projection,
              const Lighting* lighting, const glm::vec3& viewPos) const;
    bool saveScene(const std::string& filepath) const;
    bool loadScene(const std::string& filepath);
};
