#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
using namespace std;
class Shader{
    private:
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    void loadVertexShaderSource(const char* vertexPath);
    void loadFragmentShaderSource(const char* fragmentPath);
    public:
        unsigned int ID;
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();
};