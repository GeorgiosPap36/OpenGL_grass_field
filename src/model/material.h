#ifndef MATERIAL_H
#define MATERIAL_H

#include <map>
#include <variant>

#include "../shaders/shader.h"

class Material {
    public:
    Shader* shader;
    using UniformValue = std::variant<
        int,
        float,
        bool,
        glm::vec3,
        glm::mat4
    >;
    std::map<std::string, UniformValue> uniforms;

    Material(Shader* shader);

    void bindUniforms();

    void bindInt(const std::string& name, int i);
    void bindFloat(const std::string& name, float f);
    void bindBool(const std::string& name, bool b);
    void bindVec3(const std::string& name, const glm::vec3& vec3);
    void bindMat4(const std::string& name, const glm::mat4& mat4);
    
};

#endif