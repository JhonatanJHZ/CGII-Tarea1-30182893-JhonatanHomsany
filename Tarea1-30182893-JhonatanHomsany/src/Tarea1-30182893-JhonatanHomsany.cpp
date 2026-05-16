#include "../include/Tarea1-30182893-JhonatanHomsany.h"
#include "../include/UIManager.h"
#include "../include/GLFWManager.h"
#include "../include/Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

// LECTOR DE OBJ
bool loadSimpleOBJ(const char* path, std::vector<Vertex>& out_vertices) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el OBJ en: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
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
            std::string vertexData;
            std::vector<Vertex> faceVertices;

            while (iss >> vertexData) {
                std::istringstream viss(vertexData);
                std::string vIndexStr, tIndexStr, nIndexStr;

                std::getline(viss, vIndexStr, '/');
                std::getline(viss, tIndexStr, '/');
                std::getline(viss, nIndexStr, '/');

                int vIndex = std::stoi(vIndexStr) - 1;
                Vertex vertex;
                vertex.position = temp_vertices[vIndex];

                if (!nIndexStr.empty() && !temp_normals.empty()) {
                    int nIndex = std::stoi(nIndexStr) - 1;
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

int main() {
    GLFWManager* glfwManager = new GLFWManager();
    GLFWwindow* window = glfwManager->createWindow(800, 600, "Tarea 1 - Cubo Rubik");
    if (!window) {
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader* shader = new Shader("../../../Tarea1-30182893-JhonatanHomsany/shaders/default.vert", "../../../Tarea1-30182893-JhonatanHomsany/shaders/default.frag");
    unsigned int shaderProgram = shader->ID;

    std::vector<Vertex> vertices;
    // La ruta ahora apuntará a la carpeta assets de ESTE proyecto
    const char* objPath = "../../../Tarea1-30182893-JhonatanHomsany/assets/Rubik/Robik.obj";

    if (!loadSimpleOBJ(objPath, vertices)) {
        std::cerr << "Revisa la ruta del archivo Robik.obj. Ruta intentada: " << objPath << std::endl;
    }

    // --- NUEVO: CENTRAR EL MODELO MATEMÁTICAMENTE ---
    if (!vertices.empty()) {
        glm::vec3 minBounds = vertices[0].position;
        glm::vec3 maxBounds = vertices[0].position;

        // 1. Encontrar los extremos del modelo
        for (const auto& v : vertices) {
            minBounds = glm::min(minBounds, v.position);
            maxBounds = glm::max(maxBounds, v.position);
        }

        // 2. Calcular el punto central
        glm::vec3 center = (minBounds + maxBounds) / 2.0f;

        // 3. Restar el centro a todos los vértices para mover el modelo al (0,0,0)
        for (auto& v : vertices) {
            v.position -= center;
        }
    }
    // ------------------------------------------------

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Usar vertices.data() en lugar de &vertices[0] evita excepciones cuando el vector está vacío
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    UIManager* uiManager = new UIManager(window);

    float rotationSpeed = 1.0f;
    float scale = 0.1f;

    while (!glfwManager->shouldClose()) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uiManager->newFrame();
        uiManager->drawInspector();

        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * rotationSpeed, glm::vec3(0.5f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale));

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8.0f));

        int display_w, display_h;
        glfwManager->getFrameBufferSize(&display_w, &display_h);
        display_h = std::max(1, display_h);
        float aspect = (float)display_w / (float)display_h;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

        uiManager->render();
        glfwManager->update();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    delete uiManager;
    //delete shader;
    delete glfwManager;

    return 0;
}