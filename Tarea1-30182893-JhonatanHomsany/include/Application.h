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

using namespace std;

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
    void handleKeyboardEvents(float deltaTime);
};
