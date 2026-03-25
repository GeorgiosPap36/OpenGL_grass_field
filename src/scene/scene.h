#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <filesystem>

#include <glm/glm.hpp>

#include "../model/model.h"

struct SceneNode {
    std::unique_ptr<Model> model;
    std::map<std::string, SceneNode> childNodes;
    u_int modelInstances;
    bool isActive;

    void applyTransformation(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
    void render(Shader &shader, int globalVariableSize = 0);
};

class Scene {
    public:
    SceneNode rootNode; 
    
    virtual ~Scene() = default;

    virtual void render(float interPolation) = 0;
    virtual void update(float dt, std::map<int, bool>& keyboard, glm::vec2 mouseMovement) = 0;
};

#endif 