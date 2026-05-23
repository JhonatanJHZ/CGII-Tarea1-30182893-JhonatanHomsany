#include "Ray.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "SceneObject.h"

Ray::Ray(const glm::vec3 origin, const glm::vec3 direction, Color rgba, float t_min, float t_max){
    this->origin = origin;
    this->direction = direction;
    this->rgba = rgba;
    this->t_min = t_min;
    this->t_max = t_max;
    this->hit_object = nullptr;
    this->hit_t = t_max;
    this->hit_normal = glm::vec3(0.0f, 0.0f, 0.0f);
}

Ray::~Ray(){}

glm::vec3 Ray::getOrigin() const {
    return origin;
}

glm::vec3 Ray::getDirection() const {
    return direction;
}

void Ray::drawRay(const glm::mat4& view, const glm::mat4& projection, Shader* shader){
    glm::vec3 point = origin + direction * t_max;
    drawSegment(origin, point, rgba, view, projection, shader);
}

bool Ray::intersect(SceneObject *obj){
    if(obj == nullptr) return false;
    
    float t_hit;
    glm::vec3 normal;
    
    float worldRadius = obj->localRadius * std::max({obj->scale.x, obj->scale.y, obj->scale.z});
    
    if(hitSphere(obj->position, worldRadius, *this, t_hit, normal)){
        if(t_hit < hit_t && t_hit > t_min){
            hit_object = obj;
            hit_t = t_hit;
            hit_normal = normal;
            return true;
        }
    }   
    return false;
}



void Ray::drawSegment(glm::vec3 p1, glm::vec3 p2, Color rgba, const glm::mat4& view, const glm::mat4& projection, Shader* shader){
    if (!shader) return;

    float vertices[] = {
        p1.x, p1.y, p1.z, 
        p2.x, p2.y, p2.z  
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glUseProgram(shader->ID);
    
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(shader->ID, "objectColor"), rgba.r, rgba.g, rgba.b, rgba.a);
    glLineWidth(5.0f);
    glDrawArrays(GL_LINES, 0, 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

bool Ray::hitSphere(const glm::vec3& center, float radius, const Ray& ray, float& t_hit, glm::vec3& normal){
    glm::vec3 oc = center - ray.getOrigin();
    float a = glm::dot(ray.getDirection(), ray.getDirection());
    float b = -2.0f * glm::dot(ray.getDirection(), oc);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) { return false; }
    float t = (-b - sqrt(discriminant)) / (2.0f * a);
    if (t > 0.001f) {
        t_hit = t;
        glm::vec3 hit_point = ray.getOrigin() + t * ray.getDirection();
        normal = glm::normalize(hit_point - center);
        return true;
    }
    return false; 
}