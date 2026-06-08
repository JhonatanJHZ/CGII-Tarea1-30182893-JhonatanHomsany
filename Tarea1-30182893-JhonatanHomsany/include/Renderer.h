#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "Shader.h"
class GLTFManager;
class Mesh;
class Renderer {
public:
    Renderer();
    ~Renderer();
    void clear(float r = 0.1f, float g = 0.1f, float b = 0.1f, float a = 1.0f) const;
    void render(const GLTFManager* gltf, const Shader* shader, 
                const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const;
    void render(const Mesh* mesh, const Shader* shader, 
                const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const;
};
