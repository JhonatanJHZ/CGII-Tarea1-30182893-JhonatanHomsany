#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
enum class ShadingMode {
    FLAT = 0,
    LAMBERT = 1,
    PHONG = 2,
    BLINN_PHONG = 3,
    PBR = 4
};
struct Light {
    std::string name;
    glm::vec3 position = glm::vec3(2.0f, -2.8f, 5.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    float ambientIntensity = 0.15f;
    float specularStrength = 0.5f;
    float shininess = 32.0f;
};
class Lighting {
public:
    std::vector<Light> lights;
    ShadingMode activeMode = ShadingMode::FLAT;
    float exposure = 2.0f; 
    Lighting() {
        Light defaultLight;
        defaultLight.name = "Luz Principal";
        lights.push_back(defaultLight);
    }
};
