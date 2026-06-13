#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

struct TexVertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinates;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinates;
    glm::vec4 color;
};
class Mesh{
    private:
    vector<Vertex> vertices;
    unsigned int VAO, VBO;
    bool loadSimpleOBJ(const string& path, vector<Vertex>& out_vertices);
    void setupMesh();
    public:
    Mesh(const string objPath);
    Mesh(const std::vector<Vertex>& vertices);
    ~Mesh();
    void centerModel();
    void draw() const;
    vector<Vertex> getVertices();
};