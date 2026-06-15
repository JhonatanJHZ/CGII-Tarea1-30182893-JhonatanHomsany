#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <glad/glad.h>
#include <tiny_gltf_v3.h> 
#include "../Mesh.h"

struct PBRMaterial {
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    
    unsigned int baseColorTexture = 0;
    unsigned int metallicRoughnessTexture = 0;
    unsigned int normalTexture = 0;
    unsigned int occlusionTexture = 0;
    unsigned int emissiveTexture = 0;
    glm::vec3 emissiveFactor = glm::vec3(0.0f);
};

struct GLTFPrimitive {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int indexCount = 0;
    unsigned int vertexCount = 0;
    int mode = 4;
    bool hasIndices = false;
    int materialIndex = -1;
};

class GLTFManager {
public:
    tinygltf3::Model model;
    std::vector<GLTFPrimitive> primitives;
    std::vector<PBRMaterial> materials;
    std::vector<unsigned int> glTextures;
    std::vector<Vertex> cachedVertices;
    std::string modelPath;

    GLTFManager();
    ~GLTFManager();
    bool loadModel(const std::string& filename);
    void setupGL();
    void draw(const class Shader* shader = nullptr) const;
    std::vector<Vertex> getVertices() const;
};