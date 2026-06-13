#pragma once
#include <vector>
struct GLFWwindow;
class GLFWManager;
class UIManager;
class Renderer;
class Shader;
class Scene;
class Camera;
class Lighting;
class Ray;
class InputPicker;
class ShadowManager;
#include <glm/glm.hpp>
using namespace std;

struct SSBOTriangle {
    glm::vec4 v0;
    glm::vec4 v1;
    glm::vec4 v2;
    glm::vec4 normal;
    glm::vec4 color;
    float reflectivity;
    float transparency;
    float refractiveIndex;
    float metallic;
    float roughness;
    float ao;
    int textureType;
    int hasBumpMap;
    int albedoMapID;
    float pad[3];
    glm::vec4 local_v0;
    glm::vec4 local_v1;
    glm::vec4 local_v2;
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec2 uv2;
    float pad4[2];
};
class Application {
public:
    GLFWwindow* window;
    GLFWManager* glfwManager;
    UIManager* uiManager;
    Renderer* renderer;
    Shader* shader;
    Shader* flatShader;
    Shader* shadowDepthShader;
    Scene* scene;
    vector<Camera*> cameras;
    int activeCameraIndex;
    Lighting* lighting;
    Ray* ray;
    InputPicker* picker;
    bool init();
    void updateAndRender();
    void cleanup();
    void loadBoxScene();
    Application();
    ~Application();
    void run();
private:
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;
    unsigned int ssboTriangles = 0;
    Shader* raytraceShader = nullptr;
    void handleKeyboardEvents(float deltaTime);
};
