#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "UIManager.h"
#include <iostream>
using namespace std;
class GLFWManager{
    private:
        GLFWwindow* window;
    public:
    GLFWManager();
    ~GLFWManager();
    GLFWwindow* createWindow(int width, int height, const char* title);
    void getFrameBufferSize(int* width, int* height);
    void update();
    bool shouldClose();
    private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};