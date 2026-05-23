#pragma once

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

class Application {
public:
    GLFWwindow* window;
    GLFWManager* glfwManager;
    UIManager* uiManager;
    Renderer* renderer;
    Shader* shader;
    Shader* flatShader;
    Scene* scene;
    Camera* camera;
    Lighting* lighting;
    Ray* ray;
    InputPicker* picker;

    bool init();
    void updateAndRender();
    void cleanup();

    Application();
    ~Application();
    void run();
};
