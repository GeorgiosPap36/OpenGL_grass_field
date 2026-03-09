#include "camera.h"

Camera::Camera() {

};

Camera::Camera(float fovY) 
    : up(glm::vec3(0, 1, 0)), forward(glm::vec3(0, 0, -1)), position(glm::vec3(0)), pitch(0), yaw(0), fovY(fovY) {

}

Camera::Camera(glm::vec3 up, glm::vec3 forward, glm::vec3 position, float pitch, float yaw, float fovY) 
    : up(up), forward(forward), position(position), pitch(pitch), yaw(yaw), fovY(fovY) {

}

glm::mat4 Camera::viewMatrix() {
    return glm::lookAt(position, position + forward, up);
}