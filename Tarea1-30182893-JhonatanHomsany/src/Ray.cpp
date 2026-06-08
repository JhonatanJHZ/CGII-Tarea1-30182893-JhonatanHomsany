#include "Ray.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "SceneObject.h"
#include "../include/tools/GLTFManager.h"
#include "../include/Scene.h"
#include "../include/Lighting.h"
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
    glm::vec3 point = origin + direction * glm::min(t_max, hit_t);
    drawSegment(origin, point, rgba, view, projection, shader);
}
bool Ray::intersect(SceneObject *obj){
    if(obj == nullptr) return false;
    if(obj->shape == ShapeType::SPHERE){
        float t_hit;
        glm::vec3 normal;
        float radius = std::max({obj->scale.x, obj->scale.y, obj->scale.z}) * 0.5f;
        if(hitSphere(obj->position, radius, *this, t_hit, normal)){
            if(t_hit < hit_t && t_hit > t_min){
                hit_object = obj;
                hit_t = t_hit;
                hit_normal = normal;
                return true;
            }
        }
        return false;
    }
    std::vector<Vertex> localVertices;
    if (obj->type == MeshType::REVOLUTION_SOLID) {
        if (obj->meshPointer) {
            localVertices = static_cast<Mesh*>(obj->meshPointer)->getVertices();
        }
    } else if (obj->type == MeshType::GLTF) {
        if (obj->meshPointer) {
            localVertices = static_cast<GLTFManager*>(obj->meshPointer)->getVertices();
        }
    }
    if (localVertices.empty()) return false;
    bool hitAny = false;
    float closest_t = hit_t;
    glm::vec3 closest_normal = hit_normal;
    glm::mat4 modelMatrix = obj->getModelMatrix();
    for (size_t i = 0; i < localVertices.size(); i += 3) {
        glm::vec3 v0 = glm::vec3(modelMatrix * glm::vec4(localVertices[i].position, 1.0f));
        glm::vec3 v1 = glm::vec3(modelMatrix * glm::vec4(localVertices[i+1].position, 1.0f));
        glm::vec3 v2 = glm::vec3(modelMatrix * glm::vec4(localVertices[i+2].position, 1.0f));
        float t_temp;
        glm::vec3 normal_temp;
        if (hitTriangle(v0, v1, v2, *this, t_temp, normal_temp)) {
            if (t_temp < closest_t && t_temp > t_min) {
                closest_t = t_temp;
                closest_normal = normal_temp;
                hitAny = true;
            }
        }
    }
    if (hitAny) {
        hit_object = obj;
        hit_t = closest_t;
        hit_normal = closest_normal;
        return true;
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
bool Ray::hitTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const Ray& ray, float& t_hit, glm::vec3& normal) {
    const float EPSILON = 1e-6f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(ray.getDirection(), edge2);
    float a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) {
        return false; 
    }
    float f = 1.0f / a;
    glm::vec3 s = ray.getOrigin() - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.getDirection(), q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    float t = f * glm::dot(edge2, q);
    if (t > EPSILON) {
        t_hit = t;
        normal = glm::normalize(glm::cross(edge1, edge2));
        if (glm::dot(normal, ray.getDirection()) > 0.0f) {
            normal = -normal;
        }
        return true;
    }
    return false;
}
glm::vec3 Ray::Raytracing(Ray ray, Scene* scene, Lighting* lighting, int bounces) {
    if (bounces <= 0) {
        return glm::vec3(0.0f);
    }
    ray.hit_object = nullptr;
    ray.hit_t = ray.t_max;
    for(SceneObject& obj : scene->objects){
        ray.intersect(&obj);
    }
    if(ray.hit_object == nullptr) return glm::vec3(0.0f);
        glm::vec3 hitPoint = ray.getOrigin() + ray.getDirection() * ray.hit_t;
    glm::vec3 normal = ray.hit_normal;
    glm::vec3 viewDir = glm::normalize(-ray.getDirection());
    glm::vec3 directLightColor = glm::vec3(0.0f);
    glm::vec3 reflectionColor = glm::vec3(0.0f);
    glm::vec3 refractionColor = glm::vec3(0.0f);
    float reflectivity = ray.hit_object->reflectivity;
    float transparency = ray.hit_object->transparency;
    if (reflectivity > 0.0f) {
        glm::vec3 reflectedDir = glm::normalize(glm::reflect(ray.getDirection(), normal));
        Ray reflectedRay(hitPoint + normal * 0.001f, reflectedDir, Color{0,0,0,0}, 0.0f, ray.t_max);
        reflectionColor = Raytracing(reflectedRay, scene, lighting, bounces - 1);
    }
    if (transparency > 0.0f) {
        float ior = ray.hit_object->refractiveIndex;
        float eta = 1.0f / ior;
        glm::vec3 refractNormal = normal;
        if (glm::dot(ray.getDirection(), normal) > 0.0f) {
            refractNormal = -normal;
            eta = ior;
        }
        glm::vec3 refractedDir = glm::refract(ray.getDirection(), refractNormal, eta);
        if (glm::length(refractedDir) < 0.01f) {
            glm::vec3 reflectedDir = glm::normalize(glm::reflect(ray.getDirection(), refractNormal));
            Ray internalReflectedRay(hitPoint + refractNormal * 0.001f, reflectedDir, Color{0,0,0,0}, 0.0f, ray.t_max);
            refractionColor = Raytracing(internalReflectedRay, scene, lighting, bounces - 1);
        } else {
            refractedDir = glm::normalize(refractedDir);
            Ray refractedRay(hitPoint - refractNormal * 0.001f, refractedDir, Color{0,0,0,0}, 0.0f, ray.t_max);
            refractionColor = Raytracing(refractedRay, scene, lighting, bounces - 1);
        }
    }
    for(Light& light : lighting->lights){
        glm::vec3 lightDir = glm::normalize(light.position - hitPoint);
        glm::vec3 objColor = ray.hit_object->color;
        glm::vec3 ambient = light.color * 0.15f * objColor;
        directLightColor += ambient; 
        Ray shadowRay(hitPoint + normal * 0.001f, lightDir, Color{0,0,0,0}, 0.0f, glm::distance(light.position, hitPoint));
        shadowRay.hit_object = nullptr;
        shadowRay.hit_t = shadowRay.t_max;
        for(SceneObject& obj : scene->objects){
            shadowRay.intersect(&obj);
        }
        if (shadowRay.hit_object == nullptr) {
            glm::vec3 reflectedDir = glm::reflect(-lightDir, normal);
            float diff = std::max(0.0f, glm::dot(normal, lightDir));
            float spec = std::max(0.0f, glm::dot(reflectedDir, viewDir));
            spec = glm::pow(spec, 32.0f);
            glm::vec3 diffuse = light.color * diff * objColor;
            glm::vec3 specular = light.color * spec;
            directLightColor += diffuse + specular;
        }
    }
    glm::vec3 finalColor = glm::vec3(0.0f);
    float solidPercentage = 1.0f - reflectivity - transparency;
    if (solidPercentage > 0.0f) {
        finalColor += directLightColor * solidPercentage;
    }
    finalColor += reflectionColor * reflectivity;
    finalColor += refractionColor * transparency;
    finalColor.x = std::min(1.0f, finalColor.x);
    finalColor.y = std::min(1.0f, finalColor.y);
    finalColor.z = std::min(1.0f, finalColor.z);
    return finalColor;
}
