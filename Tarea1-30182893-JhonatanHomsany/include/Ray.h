#pragma once
#include <glm/glm.hpp>
#include "SceneObject.h"
#include "Shader.h"
class Scene;
class Lighting;
struct Color{
    float r, g, b, a;
};
class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;
    float t_min;
    float t_max;
    Color rgba;
    SceneObject* hit_object;
    glm::vec3 hit_normal;
    float hit_t;
    Ray(const glm::vec3 origin, const glm::vec3 direction, Color rgba, float t_min, float t_max);
    ~Ray();
    void drawRay(const glm::mat4& view, const glm::mat4& projection, Shader* shader);
    void drawSegment(glm::vec3 p1, glm::vec3 p2, Color rgba, const glm::mat4& view, const glm::mat4& projection, Shader* shader);
    bool intersect(SceneObject* obj);
    glm::vec3 getOrigin() const;
    glm::vec3 getDirection() const;
    static glm::vec3 Raytracing(Ray ray, Scene* scene, Lighting* lighting, int bounces);
    private:
    bool hitSphere(const glm::vec3& center, float radius, const Ray& ray, float& t_hit, glm::vec3& normal);
    bool hitTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const Ray& ray, float& t_hit, glm::vec3& normal);
};