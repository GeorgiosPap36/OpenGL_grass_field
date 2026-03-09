#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <map>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.1415926535897932384626433832795

class Camera {
    public:
    glm::vec3 up, worldUp, forward, right, position;
    float pitch, yaw, fovY;

    virtual ~Camera() = default;

    Camera();
    Camera(float fovY);
    Camera(glm::vec3 up, glm::vec3 forward, glm::vec3 position, float pitch, float yaw, float fovY);

    glm::mat4 viewMatrix();

    virtual void update(float dt, std::map<int, bool>& keyboard, glm::vec2 mouseMovement) = 0;
};

#endif