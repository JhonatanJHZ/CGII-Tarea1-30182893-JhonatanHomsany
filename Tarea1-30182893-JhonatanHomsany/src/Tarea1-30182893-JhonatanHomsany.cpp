#include "../include/Tarea1-30182893-JhonatanHomsany.h"

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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace std;

// --- Shaders ---
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = aNormal;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 Normal;

void main() {
    // Iluminación básica basada en la normal
    vec3 color = Normal * 0.5 + 0.5;
    FragColor = vec4(color, 1.0);
}
)";

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

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
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Tarea 1 - Cubo Rubik", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fallo al inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float rotationSpeed = 1.0f;
    float scale = 0.1f;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controles del Rubik");
        ImGui::SliderFloat("Velocidad de Rotacion", &rotationSpeed, 0.0f, 5.0f);
        ImGui::SliderFloat("Escala", &scale, 0.01f, 2.0f);
        ImGui::Text("Vertices cargados: %zu", vertices.size());
        ImGui::End();

        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * rotationSpeed, glm::vec3(0.5f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale));

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8.0f));

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        display_h = std::max(1, display_h);
        float aspect = (float)display_w / (float)display_h;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}