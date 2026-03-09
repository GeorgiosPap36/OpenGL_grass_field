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

void SceneNode::render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {

}
