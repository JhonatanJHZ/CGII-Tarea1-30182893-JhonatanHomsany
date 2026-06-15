#include "../include/Mesh.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
Mesh::Mesh(const string objPath){
    if (!loadSimpleOBJ(objPath, vertices)) {
        std::cerr << "Revisa la ruta del archivo Robik.obj. Ruta intentada: " << objPath << std::endl;
    }
    centerModel();
    setupMesh();
}
Mesh::Mesh(const std::vector<Vertex>& vertices) : vertices(vertices) {
    centerModel();
    setupMesh();
}
Mesh::~Mesh(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
void Mesh::draw() const{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
}
void Mesh::setupMesh(){
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoordinates));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}
void Mesh::centerModel(){
    if (!vertices.empty()) {
        glm::vec3 minBounds = vertices[0].position;
        glm::vec3 maxBounds = vertices[0].position;
        for (const auto& v : vertices) {
            minBounds = glm::min(minBounds, v.position);
            maxBounds = glm::max(maxBounds, v.position);
        }
        glm::vec3 center = (minBounds + maxBounds) / 2.0f;
        for (auto& v : vertices) {
            v.position -= center;
        }
    }
}
bool Mesh::loadSimpleOBJ(const string& path, vector<Vertex>& out_vertices) {
    vector<glm::vec3> temp_vertices;
    vector<glm::vec3> temp_normals;
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el OBJ en: " << path << endl;
        return false;
    }
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string type;
        iss >> type;
        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (type == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "f") {
            string vertexData;
            vector<Vertex> faceVertices;
            while (iss >> vertexData) {
                istringstream viss(vertexData);
                string vIndexStr, tIndexStr, nIndexStr;
                getline(viss, vIndexStr, '/');
                getline(viss, tIndexStr, '/');
                getline(viss, nIndexStr, '/');
                int vIndex = stoi(vIndexStr) - 1;
                Vertex vertex;
                vertex.position = temp_vertices[vIndex];
                if (!nIndexStr.empty() && !temp_normals.empty()) {
                    int nIndex = stoi(nIndexStr) - 1;
                    vertex.normal = temp_normals[nIndex];
                }
                else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                faceVertices.push_back(vertex);
            }
            for (size_t i = 1; i < faceVertices.size() - 1; i++) {
                out_vertices.push_back(faceVertices[0]);
                out_vertices.push_back(faceVertices[i]);
                out_vertices.push_back(faceVertices[i + 1]);
            }
        }
    }
    return true;
}
vector<Vertex> Mesh::getVertices(){
    return vertices;
}
