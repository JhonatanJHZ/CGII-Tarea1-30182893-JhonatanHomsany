#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Mesh.h"
class Scene;
class Lighting;
class Camera;
class BasicShapesGenerator {
public:
    static std::vector<Vertex> generateCube(float size, bool inwardNormals = false);
    static std::vector<Vertex> generateCylinder(float radius, float height, int sectors = 30);
    static std::vector<Vertex> generatePyramid(float baseSize, float height);
    static std::vector<Vertex> generateSphere(float radius, int rings = 30, int sectors = 30);
    static std::vector<Vertex> generateQuad();
    static void loadDefaultBoxScene(Scene* scene, Lighting* lighting, Camera* camera);
};
