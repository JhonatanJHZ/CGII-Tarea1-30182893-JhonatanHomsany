#include "../../include/tools/BasicShapesGenerator.h"
#include "../../include/Scene.h"
#include "../../include/Lighting.h"
#include "../../include/Camera.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
std::vector<Vertex> BasicShapesGenerator::generateCube(float size, bool inwardNormals) {
    float half = size * 0.5f;
    float normalSign = inwardNormals ? -1.0f : 1.0f;
    std::vector<Vertex> vertices = {
        { {-half, -half, -half}, {0.0f, 0.0f, -normalSign} },
        { { half,  half, -half}, {0.0f, 0.0f, -normalSign} },
        { { half, -half, -half}, {0.0f, 0.0f, -normalSign} },
        { { half,  half, -half}, {0.0f, 0.0f, -normalSign} },
        { {-half, -half, -half}, {0.0f, 0.0f, -normalSign} },
        { {-half,  half, -half}, {0.0f, 0.0f, -normalSign} },
        { {-half, -half,  half}, {0.0f, 0.0f,  normalSign} },
        { { half, -half,  half}, {0.0f, 0.0f,  normalSign} },
        { { half,  half,  half}, {0.0f, 0.0f,  normalSign} },
        { { half,  half,  half}, {0.0f, 0.0f,  normalSign} },
        { {-half,  half,  half}, {0.0f, 0.0f,  normalSign} },
        { {-half, -half,  half}, {0.0f, 0.0f,  normalSign} },
        { {-half,  half,  half}, {-normalSign, 0.0f, 0.0f} },
        { {-half,  half, -half}, {-normalSign, 0.0f, 0.0f} },
        { {-half, -half, -half}, {-normalSign, 0.0f, 0.0f} },
        { {-half, -half, -half}, {-normalSign, 0.0f, 0.0f} },
        { {-half, -half,  half}, {-normalSign, 0.0f, 0.0f} },
        { {-half,  half,  half}, {-normalSign, 0.0f, 0.0f} },
        { { half,  half,  half}, { normalSign, 0.0f, 0.0f} },
        { { half, -half, -half}, { normalSign, 0.0f, 0.0f} },
        { { half,  half, -half}, { normalSign, 0.0f, 0.0f} },
        { { half, -half, -half}, { normalSign, 0.0f, 0.0f} },
        { { half,  half,  half}, { normalSign, 0.0f, 0.0f} },
        { { half, -half,  half}, { normalSign, 0.0f, 0.0f} },
        { {-half, -half, -half}, {0.0f, -normalSign, 0.0f} },
        { { half, -half, -half}, {0.0f, -normalSign, 0.0f} },
        { { half, -half,  half}, {0.0f, -normalSign, 0.0f} },
        { { half, -half,  half}, {0.0f, -normalSign, 0.0f} },
        { {-half, -half,  half}, {0.0f, -normalSign, 0.0f} },
        { {-half, -half, -half}, {0.0f, -normalSign, 0.0f} },
        { {-half,  half, -half}, {0.0f,  normalSign, 0.0f} },
        { {-half,  half,  half}, {0.0f,  normalSign, 0.0f} },
        { { half,  half,  half}, {0.0f,  normalSign, 0.0f} },
        { { half,  half,  half}, {0.0f,  normalSign, 0.0f} },
        { { half,  half, -half}, {0.0f,  normalSign, 0.0f} },
        { {-half,  half, -half}, {0.0f,  normalSign, 0.0f} }
    };
    return vertices;
}
std::vector<Vertex> BasicShapesGenerator::generatePyramid(float baseSize, float height) {
    float halfBase = baseSize * 0.5f;
    float halfHeight = height * 0.5f;
    glm::vec3 apex(0.0f, halfHeight, 0.0f);
    glm::vec3 c0(-halfBase, -halfHeight, -halfBase);
    glm::vec3 c1( halfBase, -halfHeight, -halfBase);
    glm::vec3 c2( halfBase, -halfHeight,  halfBase);
    glm::vec3 c3(-halfBase, -halfHeight,  halfBase);
    glm::vec3 n_front = glm::normalize(glm::vec3(0.0f, halfBase, height));
    glm::vec3 n_back  = glm::normalize(glm::vec3(0.0f, halfBase, -height));
    glm::vec3 n_left  = glm::normalize(glm::vec3(-height, halfBase, 0.0f));
    glm::vec3 n_right = glm::normalize(glm::vec3(height, halfBase, 0.0f));
    glm::vec3 n_bottom(0.0f, -1.0f, 0.0f);
    std::vector<Vertex> vertices = {
        { c0, n_bottom }, { c2, n_bottom }, { c1, n_bottom },
        { c0, n_bottom }, { c3, n_bottom }, { c2, n_bottom },
        { c3, n_front }, { c2, n_front }, { apex, n_front },
        { c1, n_back }, { c0, n_back }, { apex, n_back },
        { c0, n_left }, { c3, n_left }, { apex, n_left },
        { c2, n_right }, { c1, n_right }, { apex, n_right }
    };
    return vertices;
}
std::vector<Vertex> BasicShapesGenerator::generateSphere(float radius, int rings, int sectors) {
    std::vector<Vertex> vertices;
    auto getSphereVertex = [&](float phi, float theta) -> Vertex {
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
        glm::vec3 normal(sinPhi * cosTheta, cosPhi, sinPhi * sinTheta);
        Vertex v;
        v.position = normal * radius;
        v.normal = normal;
        return v;
    };
    for (int i = 0; i < rings; ++i) {
        float phi0 = glm::pi<float>() * (float)i / (float)rings;
        float phi1 = glm::pi<float>() * (float)(i + 1) / (float)rings;
        for (int j = 0; j < sectors; ++j) {
            float theta0 = 2.0f * glm::pi<float>() * (float)j / (float)sectors;
            float theta1 = 2.0f * glm::pi<float>() * (float)(j + 1) / (float)sectors;
            Vertex v00 = getSphereVertex(phi0, theta0);
            Vertex v01 = getSphereVertex(phi0, theta1);
            Vertex v10 = getSphereVertex(phi1, theta0);
            Vertex v11 = getSphereVertex(phi1, theta1);
            vertices.push_back(v00);
            vertices.push_back(v10);
            vertices.push_back(v01);
            vertices.push_back(v01);
            vertices.push_back(v10);
            vertices.push_back(v11);
        }
    }
    return vertices;
}
std::vector<Vertex> BasicShapesGenerator::generateQuad() {
    std::vector<Vertex> vertices = {
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} }
    };
    return vertices;
}
void BasicShapesGenerator::loadDefaultBoxScene(Scene* scene, Lighting* lighting, Camera* camera) {
    if (!scene || !lighting || !camera) return;
    while (!scene->objects.empty()) {
        scene->removeObject(0);
    }
    std::vector<Vertex> floorVerts = BasicShapesGenerator::generateQuad();
    Mesh* floorMesh = new Mesh(floorVerts);
    SceneObject floorObj;
    floorObj.name = "Piso";
    floorObj.type = MeshType::REVOLUTION_SOLID;
    floorObj.shape = ShapeType::PLANE;
    floorObj.meshPointer = floorMesh;
    floorObj.position = glm::vec3(0.0f, -8.0f, 0.0f);
    floorObj.rotation = glm::vec3(-90.0f, 0.0f, 0.0f); 
    floorObj.scale = glm::vec3(16.0f, 16.0f, 1.0f);
    floorObj.color = glm::vec3(0.40f, 0.40f, 0.40f);
    floorObj.reflectivity = 0.3f;
    scene->addObject(floorObj);
    std::vector<Vertex> ceilVerts = BasicShapesGenerator::generateQuad();
    Mesh* ceilMesh = new Mesh(ceilVerts);
    SceneObject ceilObj;
    ceilObj.name = "Techo";
    ceilObj.type = MeshType::REVOLUTION_SOLID;
    ceilObj.shape = ShapeType::PLANE;
    ceilObj.meshPointer = ceilMesh;
    ceilObj.position = glm::vec3(0.0f, 8.0f, 0.0f);
    ceilObj.rotation = glm::vec3(90.0f, 0.0f, 0.0f); 
    ceilObj.scale = glm::vec3(16.0f, 16.0f, 1.0f);
    ceilObj.color = glm::vec3(0.65f, 0.65f, 0.65f);
    scene->addObject(ceilObj);
    std::vector<Vertex> leftWallVerts = BasicShapesGenerator::generateQuad();
    Mesh* leftWallMesh = new Mesh(leftWallVerts);
    SceneObject leftWallObj;
    leftWallObj.name = "Pared Izquierda";
    leftWallObj.type = MeshType::REVOLUTION_SOLID;
    leftWallObj.shape = ShapeType::PLANE;
    leftWallObj.meshPointer = leftWallMesh;
    leftWallObj.position = glm::vec3(-8.0f, 0.0f, 0.0f);
    leftWallObj.rotation = glm::vec3(0.0f, 90.0f, 0.0f); 
    leftWallObj.scale = glm::vec3(16.0f, 16.0f, 1.0f);
    leftWallObj.color = glm::vec3(0.15f, 0.35f, 0.70f);
    scene->addObject(leftWallObj);
    std::vector<Vertex> rightWallVerts = BasicShapesGenerator::generateQuad();
    Mesh* rightWallMesh = new Mesh(rightWallVerts);
    SceneObject rightWallObj;
    rightWallObj.name = "Pared Derecha";
    rightWallObj.type = MeshType::REVOLUTION_SOLID;
    rightWallObj.shape = ShapeType::PLANE;
    rightWallObj.meshPointer = rightWallMesh;
    rightWallObj.position = glm::vec3(8.0f, 0.0f, 0.0f);
    rightWallObj.rotation = glm::vec3(0.0f, -90.0f, 0.0f); 
    rightWallObj.scale = glm::vec3(16.0f, 16.0f, 1.0f);
    rightWallObj.color = glm::vec3(0.70f, 0.15f, 0.15f);
    scene->addObject(rightWallObj);
    std::vector<Vertex> frontWallVerts = BasicShapesGenerator::generateQuad();
    Mesh* frontWallMesh = new Mesh(frontWallVerts);
    SceneObject frontWallObj;
    frontWallObj.name = "Pared del Frente";
    frontWallObj.type = MeshType::REVOLUTION_SOLID;
    frontWallObj.shape = ShapeType::PLANE;
    frontWallObj.meshPointer = frontWallMesh;
    frontWallObj.position = glm::vec3(0.0f, 0.0f, -8.0f);
    frontWallObj.rotation = glm::vec3(0.0f, 0.0f, 0.0f); 
    frontWallObj.scale = glm::vec3(16.0f, 16.0f, 1.0f);
    frontWallObj.color = glm::vec3(0.75f, 0.65f, 0.15f);
    scene->addObject(frontWallObj);
    std::vector<Vertex> backWallVerts = BasicShapesGenerator::generateQuad();
    Mesh* backWallMesh = new Mesh(backWallVerts);
    SceneObject backWallObj;
    backWallObj.name = "Pared Trasera";
    backWallObj.type = MeshType::REVOLUTION_SOLID;
    backWallObj.shape = ShapeType::PLANE;
    backWallObj.meshPointer = backWallMesh;
    backWallObj.position = glm::vec3(0.0f, 0.0f, 8.0f);
    backWallObj.rotation = glm::vec3(0.0f, 180.0f, 0.0f); 
    backWallObj.scale = glm::vec3(16.0f, 16.0f, 1.0f);
    backWallObj.color = glm::vec3(0.40f, 0.40f, 0.40f);
    scene->addObject(backWallObj);
    std::vector<Vertex> cubeVerts = BasicShapesGenerator::generateCube(1.0f, false);
    Mesh* cubeMesh = new Mesh(cubeVerts);
    SceneObject cubeObj;
    cubeObj.name = "Cubo";
    cubeObj.type = MeshType::REVOLUTION_SOLID;
    cubeObj.meshPointer = cubeMesh;
    cubeObj.position = glm::vec3(-4.5f, -6.9f, 0.0f);
    cubeObj.rotation = glm::vec3(0.0f, 30.0f, 0.0f); 
    cubeObj.scale = glm::vec3(2.0f);
    cubeObj.color = glm::vec3(0.85f, 0.15f, 0.15f);
    cubeObj.transparency = 0.85f;
    cubeObj.reflectivity = 0.1f;
    cubeObj.refractiveIndex = 1.52f; 
    scene->addObject(cubeObj);
    std::vector<Vertex> pyrVerts = BasicShapesGenerator::generatePyramid(1.0f, 1.0f);
    Mesh* pyrMesh = new Mesh(pyrVerts);
    SceneObject pyrObj;
    pyrObj.name = "Piramide";
    pyrObj.type = MeshType::REVOLUTION_SOLID;
    pyrObj.meshPointer = pyrMesh;
    pyrObj.position = glm::vec3(0.0f, -6.9f, 0.0f);
    pyrObj.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    pyrObj.scale = glm::vec3(2.0f);
    pyrObj.color = glm::vec3(0.15f, 0.85f, 0.15f);
    pyrObj.reflectivity = 0.8f; 
    scene->addObject(pyrObj);
    std::vector<Vertex> sphVerts = BasicShapesGenerator::generateSphere(0.5f, 30, 30);
    Mesh* sphMesh = new Mesh(sphVerts);
    SceneObject sphObj;
    sphObj.name = "Esfera";
    sphObj.type = MeshType::REVOLUTION_SOLID;
    sphObj.shape = ShapeType::SPHERE;
    sphObj.meshPointer = sphMesh;
    sphObj.position = glm::vec3(4.5f, -6.9f, 0.0f);
    sphObj.rotation = glm::vec3(0.0f);
    sphObj.scale = glm::vec3(2.0f);
    sphObj.color = glm::vec3(41.0f / 255.0f, 69.0f / 255.0f, 69.0f / 255.0f);
    sphObj.reflectivity = 0.8f; 
    scene->addObject(sphObj);

    if (!lighting->lights.empty()) {
        Light& mainLight = lighting->lights[0];
        mainLight.position = glm::vec3(0.6f, 5.4f, 4.0f); 
        mainLight.color = glm::vec3(1.0f, 1.0f, 1.0f);     
        mainLight.intensity = 0.56f;                       
        mainLight.ambientIntensity = 0.50f;                 
        mainLight.specularStrength = 0.85f;                 
        mainLight.shininess = 48.0f;                        
    }
    
    lighting->activeMode = ShadingMode::BLINN_PHONG;
    camera->setPosition(glm::vec3(-0.20f, -4.70f, 7.20f));
    camera->setTarget(glm::vec3(-0.40f, -4.95f, 0.00f));
    camera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
    camera->setFov(85.5f);
}
