#ifndef BASIC_CAMERA_CONTROLLER_H
#define BASIC_CAMERA_CONTROLLER_H

#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "camera.h"

class BasicCameraController : public Camera {
    public:
    BasicCameraController(glm::vec3 up, glm::vec3 forward, glm::vec3 position, float pitch, float yaw, float fovY) 
        : Camera(up, forward, position, pitch, yaw, fovY) {

            worldUp = glm::vec3(0, 1, 0);
            updateCameraVectors();
    }

    void update(float dt, std::map<int, bool>& keyboard, glm::vec2 mouseMovement) override {
        glm::vec3 velocity(0.0);
        // X
        if (keyboard[GLFW_KEY_D]) {
            velocity.x = movementSpeed;
        }
        if (keyboard[GLFW_KEY_A]) {
            velocity.x = -movementSpeed;
        }
        // Y
        if (keyboard[GLFW_KEY_SPACE]) {
            velocity.y = movementSpeed;
        }
        if (keyboard[GLFW_KEY_LEFT_SHIFT]) {
            velocity.y = -movementSpeed;
        }
        // Z
        if (keyboard[GLFW_KEY_W]) {
            velocity.z = movementSpeed;
        } 
        if (keyboard[GLFW_KEY_S]) {
            velocity.z = -movementSpeed;
        }
        
        // Mouse movement
        processMouseMovement(mouseMovement.x, mouseMovement.y);

        position += dt * ((forward * velocity.z) + (up * velocity.y) + (right * velocity.x));
        // std::cout << "Position: " << glm::to_string(position) << ", Velocity: " << glm::to_string(velocity) << ", (yaw, pitch): " << "(" << yaw << ", " << pitch << ")" << std::endl;
    }

    private:
    float mouseSensitivity = 0.5;
    float movementSpeed = 10;

    void processMouseMovement(float xoffset, float yoffset) {
        if (xoffset == 0 && yoffset == 0) {
            return;
        }

        yaw += xoffset * mouseSensitivity;
        pitch += yoffset * mouseSensitivity;

        pitch = std::clamp(pitch, -89.0f, 89.0f);

        updateCameraVectors();
    }

    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        forward = glm::normalize(front);

        right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        up = glm::normalize(glm::cross(right, forward));

        // std::cout << "Up: " << glm::to_string(up) << ", forward: " << glm::to_string(forward) << ", right: " << glm::to_string(right) << std::endl;
    }
};

#endif