#pragma once

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>

#include "Mesh.h"
#include <map>
#include <unordered_map>

GLuint TextureFromFile(
    const char* path
);

// Cache global de texturas: evita recargar el mismo archivo varias veces
// cuando varios materiales distintos apuntan a la misma imagen.
extern std::unordered_map<std::string, GLuint> g_textureCache;

class Model
{
public:
    std::vector<Mesh> meshes;
    std::string directory; // <-- AGREGAR ESTO
    Model(const char* path);
    void Draw(GLuint shaderProgram);
private:
    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
};