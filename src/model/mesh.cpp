#include "mesh.h"

void Vertex::print() {
    std::cout << "pos: " << glm::to_string(position) << ", "
              << "normal: " << glm::to_string(normal) << ", "
              << "uv: " << glm::to_string(texCoords)  << std::endl;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();

    // std::cout << "Created mesh with " << vertices.size() << " vertices, " << indices.size() << " indices and " << textures.size() << std::endl;
}

void Mesh::draw(Shader &shader, int globalVariableSize)  {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for(unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        int number = 0;
        std::string texType = textures[i].type;
        if (texType == "texture_diffuse") {
            number = diffuseNr++;
        } else if (texType == "texture_specular") {
            number = specularNr++; 
        } else if (texType == "texture_normal") {
            number = normalNr++; 
        } else if (texType == "texture_height") {
            number = heightNr++;
        }
        std::string numberStr = std::to_string(globalVariableSize + number);

        // std::cout << "Texture number: " << texType + numberStr << std::endl;

        glUniform1i(glGetUniformLocation(shader.ID, (texType + numberStr).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    
    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}


void Mesh::printVertices(int n) {
    int vertSize = vertices.size();
    int size = (n <= 0 && n < vertSize) ? vertSize : n; 

    for (int i = 0; i < size; i++) {
        Vertex v = vertices[i];
        v.print();
    }
}

void Mesh::printIndices(int n) {
    int indSize = indices.size();
    int size = (n <= 0 && n < indSize) ? indSize : n; 

    for (int i = 0; i < size; i+=3) {
        std::cout << "(" << indices[i] << ", " << indices[i + 1] << ", " << indices[i + 2] << ")" << std::endl;
    }
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}