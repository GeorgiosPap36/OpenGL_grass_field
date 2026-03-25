#include "scene.h"

// SceneNode
void SceneNode::applyTransformation(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
    model->transform.position += position;
    model->transform.rotation += rotation;
    model->transform.scale += scale;

    for (auto& [key, val] : childNodes) {
        val.applyTransformation(position, rotation, scale);
    }
}

void SceneNode::render(Shader &shader, int globalVariableSize) {
    if (!isActive) {
        return;
    }

    if (modelInstances == 1) {
        model->draw(shader, globalVariableSize);
    } else if (modelInstances > 1) {
        model->drawInstanced(shader, modelInstances, globalVariableSize);
    }
}
