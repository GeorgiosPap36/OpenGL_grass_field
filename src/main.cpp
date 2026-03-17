#include <iostream>
#include <filesystem>
#include <map>

#include "../dependencies/glad/glad.h"
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "scene/grass_field_scene.cpp"


// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const std::string WINDOW_TITLE = "OpenGL_grass_field";

const double SEC_PER_UPDATE = 1.0 / 30.0;

GLFWwindow* window;

std::map<int, bool> keyboard;
glm::vec2 lastMousePos(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
glm::vec2 mouseMovement;
bool firstMouse = true;

double deltaTime = 0;
uint64_t frames = 0;
uint64_t updates = 0;

// functions
int glfwInitialization();
void update(Scene& scene);
void render(Scene& scene, float interpolation);
void displayFPSUPS(double &displayTimer);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

int main() {

    if (glfwInitialization() == -1) {
        return -1;
    }

    GrassFieldScene scene(SCR_WIDTH, SCR_HEIGHT);

    double displayTimer = 0;
    double lastCounter = glfwGetTime();
    double current;
    double lag = 0.0;

    while (!glfwWindowShouldClose(window)) {
        current = glfwGetTime();
        deltaTime = current - lastCounter;
        lastCounter = current;
        lag += deltaTime;

        while (lag >= SEC_PER_UPDATE) {
            update(scene);
            lag -= SEC_PER_UPDATE;
        }
        
        render(scene, lag / SEC_PER_UPDATE);

        displayFPSUPS(displayTimer);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

int glfwInitialization() {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Face culling 
    // glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    return 0;
}

void update(Scene& scene) {
    updates++;

    scene.update(SEC_PER_UPDATE, keyboard, mouseMovement);

    mouseMovement = glm::vec2(0.0f);
}

void render(Scene& scene, float interpolation) {
    frames++;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.render(interpolation);

    glfwSwapBuffers(window);
}

void displayFPSUPS(double &displayTimer) {
    displayTimer += deltaTime;
    if (displayTimer >= 1.0){ 
        std::string newTitle = WINDOW_TITLE + " - FPS: " + std::to_string(frames) + " - UPS: " + std::to_string(updates);
        glfwSetWindowTitle(window, newTitle.c_str());
        frames = 0;
        updates = 0;
        displayTimer = 0;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // std::cout << "Key '" << glfwGetKeyName(key, scancode) << "' has been pressed!" << std::endl;
        keyboard[key] = true;
    } else if (action == GLFW_RELEASE) {
        // std::cout << "Key '" << glfwGetKeyName(key, scancode) << "' has been released!" << std::endl;
        keyboard[key] = false;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastMousePos.x = xpos;
        lastMousePos.y = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastMousePos.x;
    float yoffset = lastMousePos.y - ypos; // reversed since y-coordinates go from bottom to top

    lastMousePos = glm::vec2(xpos, ypos);

    mouseMovement = glm::vec2(xoffset, yoffset);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}