#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include "scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "../model/material.h"

#include "../camera/basic_camera_controller.cpp"

class DemoScene : public Scene {

    struct DirectLight {
        glm::vec3 direction;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;

        glm::mat4 lightSpaceMatrix;
    };

    struct FrameUniforms {
        glm::mat4 projection;
        glm::mat4 view;

        glm::mat4 lightSpaceMatrix;
        
        glm::vec3 cameraPos;

        float _pad;
    };

    public:
    DemoScene(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) : 
        SCR_WIDTH(SCR_WIDTH), SCR_HEIGHT(SCR_HEIGHT), camera(glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(0), 0, -90, 75) {
        
        setUpScene();

        setUpRenderBatches();

        setUpFBOs();

        setUpUBOs();

        std::cout << "Loaded: 'demo_scene'!" << std::endl;
    }

    void render(float interPolation) override  {
        generateDepthMap();

        updateUBOs();
 
        glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& [material, renderBatch] : renderBatches) {
            Shader* matShader = material->shader;
            matShader->use();

            for (const auto& [name, value] : material->uniforms) {
                std::visit([&](auto&& v) {
                    matShader->set(name.c_str(), v);
                }, value);
            }

            for (const auto& model : renderBatch) {
                matShader->set("model", model->modelMatrix());
                model->draw(*matShader, GLOBAL_VARIABLE_SIZE);
            }
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        renderQuad(*renderShader, renderTexture);
    }

    void update(float dt, std::map<int, bool>& keyboard, glm::vec2 mouseMovement) override {
        camera.update(dt, keyboard, mouseMovement);
    }

    private:
    const int GLOBAL_VARIABLE_SIZE = 10;
    const int SHADOW_MAP_INDEX = 5;

    unsigned int SCR_WIDTH, SCR_HEIGHT;
    BasicCameraController camera;
    std::map<Material*, std::vector<Model*>> renderBatches;
    DirectLight dirLight;
    glm::mat4 projectionMat;

    GLuint frameUBO;

    float nearPlane = 0.1, farPlane = 200.0;
    // depthMap
    Shader* depthShader = new Shader("../src/shaders/vertex_shaders/depth_shader.vs", "../src/shaders/fragment_shaders/depth_shader.fs");
    unsigned int depthMapFBO, depthMap;
    // renderTexture
    Shader* renderShader = new Shader("../src/shaders/vertex_shaders/render_shader.vs", "../src/shaders/fragment_shaders/render_shader.fs");
    unsigned int renderFBO, renderDepthRBO, renderTexture;

    void setUpScene() {
        projectionMat = glm::perspective(glm::radians(camera.fovY), (float) SCR_WIDTH / (float) SCR_HEIGHT, nearPlane, farPlane);

        dirLight.direction = glm::vec3(0.0, -0.25, -0.75);
        dirLight.ambient = glm::vec3(0.05f); 
        dirLight.diffuse = glm::vec3(0.4f); 
        dirLight.specular = glm::vec3(0.5f);

        SceneNode testNode;

        testNode.model = std::make_unique<Model>(std::filesystem::path("../assets/models/cube/Cube.obj").string().c_str());
        testNode.model->transform.position = glm::vec3(0, 0, 0);
        testNode.model->transform.scale = glm::vec3(1.0, 1.0, 1.0);
        testNode.model->transform.rotation = glm::vec3(0, 0, 0);

        SceneNode cubeNode;
        cubeNode.model = std::make_unique<Model>(std::filesystem::path("../assets/models/cube/Cube.obj").string().c_str());

        cubeNode.model->transform.position = glm::vec3(0, -7, 0);
        cubeNode.model->transform.scale = glm::vec3(100, 0.05, 100);
        cubeNode.model->transform.rotation = glm::vec3(0, 0, 0);

        rootNode.childNodes.emplace("testNode", std::move(testNode));
        rootNode.childNodes.emplace("cubeNode", std::move(cubeNode));
    }

    void setUpRenderBatches() {
        auto& testNodeRef = rootNode.childNodes["testNode"];
        auto& cubeNodeRef = rootNode.childNodes["cubeNode"];

        Shader* shader = new Shader("../src/shaders/vertex_shaders/lighting_shadow_shader.vs", "../src/shaders/fragment_shaders/lighting_shadow_shader.fs");

        // Solid color Material
        Material* solidColorMaterial = new Material(shader);

        solidColorMaterial->bindBool("useTexture", false);
        solidColorMaterial->bindVec3("dirLight.direction", dirLight.direction);
        solidColorMaterial->bindVec3("dirLight.ambient", dirLight.ambient);
        solidColorMaterial->bindVec3("dirLight.diffuse", dirLight.diffuse);
        solidColorMaterial->bindVec3("dirLight.specular", dirLight.specular);
        solidColorMaterial->bindVec3("color", glm::vec3(1.0));
        solidColorMaterial->bindInt("shadowMap", SHADOW_MAP_INDEX);

        renderBatches[solidColorMaterial].push_back(testNodeRef.model.get());
        renderBatches[solidColorMaterial].push_back(cubeNodeRef.model.get());
    }

    void setUpFBOs() {
        // Depth
        glGenFramebuffers(1, &depthMapFBO);
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render
        // Generate depth Render Buffer Object that will hold the depth buffer values for the render frame buffer
        glGenRenderbuffers(1, &renderDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, renderDepthRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);

        // Generate the Frame Buffer Object and the texture that will be rendered on the renderQuad
        glGenFramebuffers(1, &renderFBO);
        glGenTextures(1, &renderTexture);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

        // Bind the renderTexture and renderDepthRBO on the renderFBO
        glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderDepthRBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void generateDepthMap() {
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);

        glm::vec3 lightPos = -dirLight.direction * 10.0f;
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

        dirLight.lightSpaceMatrix = lightProjection * lightView;

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); 
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT); 

        depthShader->use();
        depthShader->set("lightSpaceMatrix", dirLight.lightSpaceMatrix);

        for (const auto& [material, renderBatch] : renderBatches) {
            for (const auto& model : renderBatch) {
                depthShader->set("model", model->modelMatrix());
                model->draw(*depthShader);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_INDEX);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    void renderQuad(Shader& shader, unsigned int renderTexture) {
        static unsigned int quadVAO = 0, quadVBO;
        if (quadVAO == 0) {
            float quadVertices[] = {
                -1.0, -1.0, 0.0, 0.0, 0.0,
                1.0, -1.0, 0.0, 1.0, 0.0,
                1.0,  1.0, 0.0, 1.0, 1.0,
                -1.0,  1.0, 0.0, 0.0, 1.0
            };
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }

        shader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        shader.set("renderTexture", 0);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void setUpUBOs() {
        glGenBuffers(1, &frameUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, frameUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(FrameUniforms), nullptr, GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, 3, frameUBO);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void updateUBOs() {
        FrameUniforms data;
        data.projection = projectionMat;
        data.view = camera.viewMatrix();
        data.lightSpaceMatrix = dirLight.lightSpaceMatrix;
        data.cameraPos = camera.position;

        glBindBuffer(GL_UNIFORM_BUFFER, frameUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FrameUniforms), &data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

};

#endif