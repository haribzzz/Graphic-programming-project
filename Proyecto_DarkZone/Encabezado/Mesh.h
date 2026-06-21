#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    // Datos de skinning
    int   boneIDs[MAX_BONE_INFLUENCE] = { -1,-1,-1,-1 };
    float weights[MAX_BONE_INFLUENCE] = { 0, 0, 0, 0 };
};

struct Texture {
    GLuint      id;
    std::string type;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    GLuint VAO, VBO, EBO;

    Mesh(std::vector<Vertex>       vertices,
        std::vector<unsigned int> indices,
        std::vector<Texture>      textures);

    void Draw(GLuint shaderProgram);
};