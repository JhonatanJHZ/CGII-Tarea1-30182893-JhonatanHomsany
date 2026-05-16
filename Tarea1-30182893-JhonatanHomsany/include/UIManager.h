#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstddef>

struct GLFWwindow;

class UIManager {
    public:
        UIManager(GLFWwindow* window);
        ~UIManager();
        void newFrame();
        void drawInspector();
        void render();
};