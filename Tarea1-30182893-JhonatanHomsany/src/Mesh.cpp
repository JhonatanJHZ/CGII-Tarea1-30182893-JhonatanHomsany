#include "../include/Mesh.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Mesh::Mesh(){
    
}

Mesh::~Mesh(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
