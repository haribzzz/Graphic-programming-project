#include "Mesh.h"
#include <string>
#include <glad/glad.h>

Mesh::Mesh(
    std::vector<Vertex> vertices,
    std::vector<unsigned int> indices,
    std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW
    );

    // location 0 — Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)0
    );

    // location 1 — Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, Normal)
    );

    // location 2 — TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, TexCoords)
    );

    // location 3 — boneIDs (enteros, usar IPointer)
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(
        3, 4, GL_INT,
        sizeof(Vertex),
        (void*)offsetof(Vertex, boneIDs)
    );

    // location 4 — weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, 4, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, weights)
    );

    glBindVertexArray(0);
}

void Mesh::Draw(GLuint shaderProgram)
{
    bool multi = textures.size() > 1;

    glUniform1i(
        glGetUniformLocation(shaderProgram, "useMultiTexture"),
        multi ? 1 : 0
    );

    if (textures.size() > 0)
    {
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);

            std::string uniformName =
                "texture" + std::to_string(i + 1);

            glUniform1i(
                glGetUniformLocation(
                    shaderProgram,
                    uniformName.c_str()
                ),
                i
            );
        }
    }

    glBindVertexArray(VAO);
    glDrawElements(
        GL_TRIANGLES,
        (GLsizei)indices.size(),
        GL_UNSIGNED_INT,
        0
    );
    glBindVertexArray(0);

    // Limpiar slots extra
    for (unsigned int i = 1; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);

        std::string uniformName =
            "texture" + std::to_string(i + 1);

        glUniform1i(
            glGetUniformLocation(
                shaderProgram,
                uniformName.c_str()
            ),
            0
        );
    }

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(
        glGetUniformLocation(shaderProgram, "useMultiTexture"),
        0
    );
}