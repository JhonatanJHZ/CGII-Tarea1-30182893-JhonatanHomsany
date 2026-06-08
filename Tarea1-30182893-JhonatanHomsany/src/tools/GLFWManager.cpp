#include "../../include/tools/GLFWManager.h"
using namespace std;
GLFWManager::GLFWManager(){
    if (!glfwInit()) {
        cerr << "Fallo al inicializar GLFW" << endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
}
GLFWManager::~GLFWManager(){
    glfwTerminate();
}
void GLFWManager::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
GLFWwindow* GLFWManager::createWindow(int width, int height, const char* title){
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Fallo al inicializar GLAD" << endl;
        return nullptr;
    }
    glfwSetFramebufferSizeCallback(window, GLFWManager::framebuffer_size_callback);
    return window;
}
void GLFWManager::getFrameBufferSize(int* width, int* height) {
    glfwGetFramebufferSize(window, width, height);
}
void GLFWManager::update() {
    glfwSwapBuffers(window);
    glfwPollEvents();
}
bool GLFWManager::shouldClose() {
    return glfwWindowShouldClose(window);
}