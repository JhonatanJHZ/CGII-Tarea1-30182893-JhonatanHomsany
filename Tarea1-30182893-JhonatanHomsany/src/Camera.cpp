#include "../include/Camera.h"
Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov, float nearPlane, float farPlane)
    : position(position), target(target), up(up), fov(fov), nearPlane(nearPlane), farPlane(farPlane),
      movementSpeed(5.0f), rotationSpeed(0.15f), yaw(-90.0f), pitch(0.0f) {
    initAnglesFromTarget();
}
Camera::~Camera() {}
void Camera::initAnglesFromTarget() {
    glm::vec3 front = glm::normalize(target - position);
    pitch = glm::degrees(asin(front.y));
    yaw = glm::degrees(atan2(front.z, front.x));
}
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}
glm::mat4 Camera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}
void Camera::movement(MovementDirection direction, float deltaTime) {
    glm::vec3 front = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(front, up));
    float velocity = movementSpeed * deltaTime;
    switch (direction) {
        case MovementDirection::FORWARD: 
            position += front * velocity;
            target += front * velocity;
            break;
        case MovementDirection::BACKWARD: 
            position -= front * velocity;
            target -= front * velocity;
            break;
        case MovementDirection::LEFT: 
            position -= right * velocity;
            target -= right * velocity;
            break;
        case MovementDirection::RIGHT: 
            position += right * velocity;
            target += right * velocity;
            break;
        case MovementDirection::UP: 
            position += up * velocity;
            target += up * velocity;
            break;
        case MovementDirection::DOWN: 
            position -= up * velocity;
            target -= up * velocity;
            break;
    }
}
void Camera::rotate(float yawOffset, float pitchOffset) {
    yaw += yawOffset * rotationSpeed;
    pitch += pitchOffset * rotationSpeed;
    pitch = std::max(-89.0f, std::min(89.0f, pitch));
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    target = position + front;
}