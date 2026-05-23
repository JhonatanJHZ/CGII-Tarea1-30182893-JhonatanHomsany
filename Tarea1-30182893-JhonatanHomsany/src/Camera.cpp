#include "../include/Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov, float nearPlane, float farPlane)
    : position(position), target(target), up(up), fov(fov), nearPlane(nearPlane), farPlane(farPlane) {}

Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}
