#include "../include/Renderer.h"
#include "../include/tools/GLTFManager.h"
#include "../include/Mesh.h"
Renderer::Renderer(){
    glEnable(GL_DEPTH_TEST);
}
Renderer::~Renderer(){
}
void Renderer::clear(float r, float g, float b, float a) const{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Renderer::render(const GLTFManager* gltf, const Shader* shader, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const {
    if (!gltf || !shader) return;
    glUseProgram(shader->ID);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    gltf->draw(shader);
}
void Renderer::render(const Mesh* mesh, const Shader* shader, const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const {
    if (!mesh || !shader) return;
    glUseProgram(shader->ID);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    mesh->draw();
}
