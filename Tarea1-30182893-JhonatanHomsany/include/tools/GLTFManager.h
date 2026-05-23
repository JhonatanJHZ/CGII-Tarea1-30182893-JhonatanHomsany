#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <glad/glad.h>
#include <tiny_gltf_v3.h> 
#include "../Mesh.h"

struct GLTFPrimitive {
    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
    unsigned int indexCount = 0;
    unsigned int vertexCount = 0;
    int mode = 4; // default GL_TRIANGLES
    bool hasIndices = false;
};

class GLTFManager {
public:
    tinygltf3::Model model;
    std::vector<GLTFPrimitive> primitives;

    GLTFManager();
    ~GLTFManager();

    bool loadModel(const std::string& filename);
    void setupGL();
    void draw() const;
};