#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>

#include "../../dependencies/glad/glad.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#include "../shaders/shader.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 position, normal;
    glm::vec2 texCoords;
    glm::vec3 tangent, bitangent;

	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];

    void print();
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
    public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    void draw(Shader &shader, int globalVariableSize = 0);
    void printVertices(int n = -1);
    void printIndices(int n = -1);

    private:
    unsigned int VBO, EBO;

    void setupMesh();
};

#endif