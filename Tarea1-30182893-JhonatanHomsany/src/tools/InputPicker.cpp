#include "../../include/tools/InputPicker.h"
#include <glm/gtc/type_ptr.hpp> 
#include <algorithm>
#include <iostream>
glm::vec3 getMouseWorldPos(double mouseX, double mouseY, int width, int height, const glm::mat4& view, const glm::mat4& proj, float depthZ) {
    float x = (float)mouseX;
    float y = (float)height - (float)mouseY;
    glm::vec3 screenPos = glm::vec3(x, y, depthZ);
    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, (float)width, (float)height);
    return glm::unProject(screenPos, view, proj, viewport);
}
InputPicker::InputPicker(){
    ray = nullptr;
    hit_object = nullptr;
    hit_t = 0.0f;
}
InputPicker::~InputPicker(){
    delete ray;
    ray = nullptr;
}
Ray* InputPicker::getRay(){
    return ray;
}
SceneObject* InputPicker::getHitObject(){
    return hit_object;
}
void InputPicker::pick(double mouseX, double mouseY, int width, int height, const glm::mat4& view, const glm::mat4& proj, Scene* scene){
    if(scene == nullptr) return;
    glm::vec3 origin = getMouseWorldPos(mouseX, mouseY, width, height, view, proj, 0.0f);
    glm::vec3 farPoint = getMouseWorldPos(mouseX, mouseY, width, height, view, proj, 1.0f);
    glm::vec3 direction = glm::normalize(farPoint - origin);
    this->ray = new Ray(origin, direction, Color{1.0f, 0.0f, 0.0f, 1.0f}, 0.0f, 10.0f);
    SceneObject* previous_selection = this->hit_object;
    if(ray){
        delete ray;
    }
    ray = new Ray(origin, direction, Color{1.0f, 0.0f, 0.0f, 1.0f}, 0.1f, 100.0f);
    hit_object = nullptr;
    ray->hit_object = nullptr;
    ray->hit_t = ray->t_max;
    for (SceneObject& obj : scene->objects) {
        if(ray->intersect(&obj)){
            cout << "Input Picker ha detectado un objeto." << endl;
        }
    }
    if (ray->hit_object) {
        if (ray->hit_object == previous_selection) {
            hit_object = nullptr;
            cout << "Objeto deseleccionado." << endl;
        } else {
            hit_object = ray->hit_object;
            hit_t = ray->hit_t;
        }
    } else {
        hit_object = nullptr;
    }
}
