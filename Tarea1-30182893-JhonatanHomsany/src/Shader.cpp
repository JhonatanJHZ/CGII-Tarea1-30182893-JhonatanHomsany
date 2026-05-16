#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    loadVertexShaderSource(vertexPath);
    loadFragmentShaderSource(fragmentPath);

    const char* vShaderCode = vertexShaderSource.c_str();
    const char* fShaderCode = fragmentShaderSource.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::loadVertexShaderSource(const char* vertexPath){
    std::ifstream vShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        vShaderFile.close();
        vertexShaderSource = vShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }
}

void Shader::loadFragmentShaderSource(const char* fragmentPath){
    std::ifstream fShaderFile;
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        fShaderFile.open(fragmentPath);
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        fShaderFile.close();
        fragmentShaderSource = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }
}

Shader::~Shader() {
    glDeleteProgram(ID);    
}
