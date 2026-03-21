#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include "scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include <cstdlib>

#include "../model/material.h"

#include "../camera/basic_camera_controller.cpp"

class GrassFieldScene : public Scene {

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

    struct alignas(16) InstancedData {
        glm::vec4 position;
        glm::vec4 color;

        InstancedData(glm::vec4 position, glm::vec4 color) : position(position), color(color) {

        }
    };

    public:
    GrassFieldScene(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) : 
        SCR_WIDTH(SCR_WIDTH), SCR_HEIGHT(SCR_HEIGHT), camera(glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(0), 0, -90, 75) {
        
        setUpScene();

        setUpSkybox();

        setUpRenderBatches();

        setUpFBOs();

        setUpUBOs();

        std::cout << "Loaded: 'grass_field'!" << std::endl;
    }

    void render(float interPolation) override  {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INSTANCED_DATA_SSBO_INDEX, instancedDataSSBO);

        // generateDepthMap();

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

            for (const auto& sceneNode : renderBatch) {
                matShader->set("model", sceneNode->model->modelMatrix());
                sceneNode->render(*matShader, GLOBAL_VARIABLE_SIZE);
            }
        }

        glDisable(GL_CULL_FACE);
        renderSkybox();
        glEnable(GL_CULL_FACE);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        renderQuad(*renderShader, renderTexture);
    }

    void update(float dt, std::map<int, bool>& keyboard, glm::vec2 mouseMovement) override {
        camera.update(dt, keyboard, mouseMovement);
    }

    private:
    const int GLOBAL_VARIABLE_SIZE = 10;
    const int SHADOW_MAP_INDEX = 5;
    const int INSTANCED_DATA_SSBO_INDEX = 4;

    // Instanced data
    GLuint instancedDataSSBO;
    std::vector<InstancedData> instancedData;
    int instancedDataSize = 2250000, instancedDataRows = 1500;

    unsigned int SCR_WIDTH, SCR_HEIGHT;
    BasicCameraController camera;
    std::map<Material*, std::vector<SceneNode*>> renderBatches;
    DirectLight dirLight;
    glm::mat4 projectionMat;

    GLuint frameUBO;

    float nearPlane = 0.1, farPlane = 500.0;
    // depthMap
    Shader* depthShader = new Shader("../src/shaders/vertex_shaders/depth_shader.vs", "../src/shaders/fragment_shaders/depth_shader.fs");
    unsigned int depthMapFBO, depthMap;
    // renderTexture
    Shader* renderShader = new Shader("../src/shaders/vertex_shaders/render_shader.vs", "../src/shaders/fragment_shaders/render_shader.fs");
    unsigned int renderFBO, renderDepthRBO, renderTexture;
    // skybox
    Shader* skyboxShader = new Shader("../src/shaders/vertex_shaders/skybox_shader.vs", "../src/shaders/fragment_shaders/skybox_shader.fs");
    unsigned int skyboxVAO, skyBoxTexture;

    void setUpScene() {
        projectionMat = glm::perspective(glm::radians(camera.fovY), (float) SCR_WIDTH / (float) SCR_HEIGHT, nearPlane, farPlane);

        dirLight.direction = glm::normalize(glm::vec3(0.15, -1, -0.35));
        dirLight.ambient = glm::vec3(0.25f); 
        dirLight.diffuse = glm::vec3(0.5f); 
        dirLight.specular = glm::vec3(0.1f);

        SceneNode floorNode;
        floorNode.model = std::make_unique<Model>(std::filesystem::path("../assets/models/flat_plane/flat_plane.obj").string().c_str());

        floorNode.model->transform.position = glm::vec3(0, -2, 0);
        floorNode.model->transform.scale = glm::vec3(100, 0.05, 100);
        floorNode.model->transform.rotation = glm::vec3(0, 0, 0);
        floorNode.modelInstances = 1;

        SceneNode instancesNode;

        instancesNode.model = std::make_unique<Model>(std::filesystem::path("../assets/models/grass_blade/grass_blade.obj").string().c_str());
        instancesNode.model->transform.position = glm::vec3(0, -2, 0);
        instancesNode.model->transform.scale = glm::vec3(0.5, 0.25, 0.5);
        instancesNode.model->transform.rotation = glm::vec3(0, 0, 0);
        instancesNode.modelInstances = instancedDataSize;

        // CREATE SSBO
        int instancedDataCols = instancedDataSize / instancedDataRows;
        for (int i = -instancedDataRows / 2; i < instancedDataRows / 2; i++) {
            for (int j = -instancedDataCols / 2; j < instancedDataCols / 2; j++) {
                float rowPercentage = (i + ((float) instancedDataRows / 2)) / (float) instancedDataRows;
                float colPercentage = (j + ((float) instancedDataCols / 2)) / (float) instancedDataCols;

                float rX = rand() / (float) RAND_MAX;
                float rY = rand() / (float) RAND_MAX;
                float rZ = rand() / (float) RAND_MAX;

                glm::vec4 color = glm::vec4(rowPercentage, 0.5, colPercentage, 0);
                glm::vec4 pos = glm::vec4(
                    (rowPercentage * floorNode.model->transform.scale.x - (1 - rowPercentage) * floorNode.model->transform.scale.x + 0.1 * (1 - rX)) / instancesNode.model->transform.scale.x, 
                    -0.3 * (1 - rY) / instancesNode.model->transform.scale.y, 
                    (colPercentage * floorNode.model->transform.scale.z - (1 - colPercentage) * floorNode.model->transform.scale.z +  0.1 * (1 - rZ)) / instancesNode.model->transform.scale.z, 
                    0
                );

                InstancedData dt(pos, color);
                instancedData.push_back(dt);
            }
        }

        glGenBuffers(1, &instancedDataSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancedDataSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(InstancedData) * instancedData.size(), instancedData.data(), GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INSTANCED_DATA_SSBO_INDEX, instancedDataSSBO);

        rootNode.childNodes.emplace("instancesNode", std::move(instancesNode));
        rootNode.childNodes.emplace("floorNode", std::move(floorNode));
    }

    void setUpRenderBatches() {
        auto& instancesNodeRef = rootNode.childNodes["instancesNode"];
        auto& floorNodeRef = rootNode.childNodes["floorNode"];

        // Solid color Material
        Shader* instancedShader = new Shader("../src/shaders/vertex_shaders/lighting_shadow_shader_instanced.vs", "../src/shaders/fragment_shaders/lighting_shadow_shader_instanced.fs");
        Material* solidColorMaterialInstnced = new Material(instancedShader);

        solidColorMaterialInstnced->bindBool("useTexture", false);
        solidColorMaterialInstnced->bindVec3("dirLight.direction", dirLight.direction);
        solidColorMaterialInstnced->bindVec3("dirLight.ambient", dirLight.ambient);
        solidColorMaterialInstnced->bindVec3("dirLight.diffuse", dirLight.diffuse);
        solidColorMaterialInstnced->bindVec3("dirLight.specular", dirLight.specular);
        solidColorMaterialInstnced->bindVec3("color", glm::vec3(1.0));
        solidColorMaterialInstnced->bindInt("shadowMap", SHADOW_MAP_INDEX);

        renderBatches[solidColorMaterialInstnced].push_back(&instancesNodeRef);

        // Solid color Material
        Shader* shader = new Shader("../src/shaders/vertex_shaders/lighting_shadow_shader.vs", "../src/shaders/fragment_shaders/lighting_shadow_shader.fs");
        Material* solidColorMaterial = new Material(shader);

        solidColorMaterial->bindBool("useTexture", false);
        solidColorMaterial->bindVec3("dirLight.direction", dirLight.direction);
        solidColorMaterial->bindVec3("dirLight.ambient", dirLight.ambient);
        solidColorMaterial->bindVec3("dirLight.diffuse", dirLight.diffuse);
        solidColorMaterial->bindVec3("dirLight.specular", dirLight.specular);
        solidColorMaterial->bindVec3("color", glm::vec3(1.0));
        solidColorMaterial->bindInt("shadowMap", SHADOW_MAP_INDEX);

        renderBatches[solidColorMaterial].push_back(&floorNodeRef);
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
        glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);

        glm::vec3 lightPos = -dirLight.direction * 10.0f;
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

        dirLight.lightSpaceMatrix = lightProjection * lightView;

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); 
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT); 

        depthShader->use();
        depthShader->set("lightSpaceMatrix", dirLight.lightSpaceMatrix);

        for (const auto& [material, renderBatch] : renderBatches) {
            for (const auto& sceneNode : renderBatch) {
                depthShader->set("model", sceneNode->model->modelMatrix());
                sceneNode->render(*depthShader);
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

    void setUpSkybox() {
        float skyboxVertices[] = {
            -1.0, -1.0,  1.0,
             1.0, -1.0,  1.0,
             1.0, -1.0, -1.0,
            -1.0, -1.0, -1.0,
            -1.0,  1.0,  1.0,
             1.0,  1.0,  1.0,
             1.0,  1.0, -1.0,
            -1.0,  1.0, -1.0
        };

        unsigned int skyBoxIndices[] = {
            // Right
            1, 2, 6,
            6, 5, 1,
            // Left
            0, 4, 7,
            7, 3, 0,
            // Top
            4, 5, 6,
            6, 7, 4,
            // Bottom
            0, 3, 2,
            2, 1, 0,
            // Back
            0, 1, 5,
            5, 4, 0,
            // Front
            3, 7, 6,
            6, 2, 3
        };

        unsigned int skyboxVBO, skyBoxEBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glGenBuffers(1, &skyBoxEBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyBoxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyBoxIndices), skyBoxIndices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        std::string facesCubemap[6] = {
            std::filesystem::path("../assets/textures/skybox_cube_map/px.png").string().c_str(),
            std::filesystem::path("../assets/textures/skybox_cube_map/nx.png").string().c_str(),
            std::filesystem::path("../assets/textures/skybox_cube_map/py.png").string().c_str(),
            std::filesystem::path("../assets/textures/skybox_cube_map/ny.png").string().c_str(),
            std::filesystem::path("../assets/textures/skybox_cube_map/pz.png").string().c_str(),
            std::filesystem::path("../assets/textures/skybox_cube_map/nz.png").string().c_str(),
        };

        glGenTextures(1, &skyBoxTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        stbi_set_flip_vertically_on_load(false);
        for (unsigned int i = 0; i < 6; i++) {
            int width, height, nrChannels;
            unsigned char* data = stbi_load(facesCubemap[i].c_str(), & width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0,
                    GL_RGBA,
                    width,
                    height,
                    0,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    data
                );
                stbi_image_free(data);
            } else {
                std::cout << "Failed to load texture" << facesCubemap[i] << std::endl;
                stbi_image_free(data);

            }
        }
    }

    void renderSkybox() {
        skyboxShader->use();
        skyboxShader->set("skybox", 0);

        glDepthFunc(GL_LEQUAL);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.viewMatrix()));

        skyboxShader->set("view", skyboxView);
        skyboxShader->set("projection", projectionMat);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexture);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);
    }

};

#endif