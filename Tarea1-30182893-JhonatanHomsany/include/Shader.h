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
    std::string geometryShaderSource;
    void loadVertexShaderSource(const char* vertexPath);
    void loadFragmentShaderSource(const char* fragmentPath);
    void loadGeometryShaderSource(const char* geometryPath);
    
    public:
        unsigned int ID;
        Shader(const char* vertexPath, const char* fragmentPath);
        ~Shader();
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);

};