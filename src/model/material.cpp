#include "material.h"

Material::Material(Shader* shader) : shader(shader) {
    
}

void Material::bindUniforms() {
    shader->use();

    for (const auto& [name, value] : uniforms) {
        std::visit([&](auto&& v) {
            shader->set(name.c_str(), v);
        }, value);
    }
}

void Material::bindInt(const std::string& name, int i) {
    uniforms[name] = i;
}

void Material::bindFloat(const std::string& name, float f) {
    uniforms[name] = f;
}

void Material::bindBool(const std::string& name, bool b) {
    uniforms[name] = b;
}


void Material::bindVec3(const std::string& name, const glm::vec3& vec3) {
    uniforms[name] = vec3;
}

void Material::bindMat4(const std::string& name, const glm::mat4& mat4) {
    uniforms[name] = mat4;
}