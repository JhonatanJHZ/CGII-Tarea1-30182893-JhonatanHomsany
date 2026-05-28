#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Mesh.h"

class Scene;
class Lighting;
class Camera;

class BasicShapesGenerator {
public:
    // Genera un cubo centrado en (0, 0, 0)
    static std::vector<Vertex> generateCube(float size, bool inwardNormals = false);

    // Genera una pirámide de base cuadrada centrada en (0, 0, 0)
    static std::vector<Vertex> generatePyramid(float baseSize, float height);

    // Genera una esfera UV procedimental centrada en (0, 0, 0)
    static std::vector<Vertex> generateSphere(float radius, int rings = 30, int sectors = 30);

    // Genera un plano (quad) unitario centrado en (0, 0, 0) en el plano X-Y
    static std::vector<Vertex> generateQuad();

    // Carga la escena por defecto con la caja y las tres figuras 3D
    static void loadDefaultBoxScene(Scene* scene, Lighting* lighting, Camera* camera);
};
