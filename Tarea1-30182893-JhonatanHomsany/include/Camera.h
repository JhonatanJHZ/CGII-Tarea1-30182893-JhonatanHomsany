#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
enum class MovementDirection{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};
enum class RenderMode{
    RASTERIZATION,
    RAY_TRACING
};
class Camera {
private:
    RenderMode mode = RenderMode::RASTERIZATION;
    int rayBounces = 3;
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    float fov;
    float nearPlane;
    float farPlane;
    float movementSpeed;
    float rotationSpeed;
    float yaw;
    float pitch;
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 8.0f), 
           glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float fov = 45.0f,
           float nearPlane = 0.1f,
           float farPlane = 100.0f);
    ~Camera();
    void initAnglesFromTarget();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect) const;
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { 
        position = pos; 
        initAnglesFromTarget(); 
    }
    glm::vec3 getTarget() const { return target; }
    void setTarget(const glm::vec3& tgt) { 
        target = tgt; 
        initAnglesFromTarget(); 
    }
    glm::vec3 getUp() const { return up; }
    void setUp(const glm::vec3& u) { up = u; }
    float getFov() const { return fov; }
    void setFov(float f) { fov = f; }
    RenderMode getRenderMode()const {return mode; }
    void setRenderMode(RenderMode mode) {this->mode = mode; }
    int getRayBounces() const { return rayBounces; }
    void setRayBounces(int bounces) { rayBounces = bounces; }
    void movement(MovementDirection direction, float deltaTime);
    void rotate(float yawOffset, float pitchOffset);
};
