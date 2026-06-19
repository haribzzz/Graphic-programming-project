
#include "Model.h"
#include <iostream>
#include <fstream>
#include <stb_image.h>

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

// TextureFromFile
GLuint TextureFromFile(const char* path)
{
    std::cout << "TextureFromFile: " << path << std::endl;

    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;

        if (nrChannels == 1) { internalFormat = GL_RED;  dataFormat = GL_RED; }
        else if (nrChannels == 3) { internalFormat = GL_RGB;  dataFormat = GL_RGB; }
        else { internalFormat = GL_RGBA; dataFormat = GL_RGBA; }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
            width, height, 0,
            dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Anisotropic filtering para que las texturas de suelo y pared no se vean borrosas
        float maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        if (maxAniso > 0.0f)
            glTexParameterf(GL_TEXTURE_2D, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                glm::min(maxAniso, 8.0f));

        std::cout << "  -> OK " << width << "x" << height << " ch=" << nrChannels << std::endl;
    }
    else
    {
        std::cout << "  -> ERROR: " << stbi_failure_reason() << std::endl;
        unsigned char magenta[] = { 255, 0, 255 };
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, magenta);
    }

    stbi_image_free(data);
    return textureID;
}

// Helpers de nombre de archivo

static std::string extractFilename(const std::string& raw)
{
    // 1. Si empieza con '-', el string tiene opciones MTL; tomamos el último token
    std::string result = raw;
    if (!result.empty() && result[0] == '-')
    {
        size_t lastSpace = result.find_last_of(' ');
        if (lastSpace != std::string::npos)
            result = result.substr(lastSpace + 1);
    }

    // 2. Quitar path / directorio
    size_t slashPos = result.find_last_of("/\\");
    if (slashPos != std::string::npos)
        result = result.substr(slashPos + 1);

    // 3. Limpiar espacios/tabs al inicio y al final
    size_t s = result.find_first_not_of(" \t\r\n");
    size_t e = result.find_last_not_of(" \t\r\n");
    if (s == std::string::npos) return "";
    return result.substr(s, e - s + 1);
}

// Model
Model::Model(const char* path)
{
    loadModel(path);
}

void Model::Draw(GLuint shaderProgram)
{
    for (auto& m : meshes)
        m.Draw(shaderProgram);
}

void Model::loadModel(std::string path)
{
    std::cout << "==== Cargando modelo: " << path << " ====" << std::endl;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |          
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace
    );

    if (!scene || !scene->mRootNode)
    {
        std::cout << "ERROR ASSIMP: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    std::cout << "Directorio base: " << directory << std::endl;

    processNode(scene->mRootNode, scene);
    std::cout << "Modelo cargado: " << meshes.size() << " meshes." << std::endl;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    // ?? Vértices ??
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.Position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z);

        if (mesh->HasNormals())
            vertex.Normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
            vertex.TexCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y);
        else
            vertex.TexCoords = glm::vec2(0.0f);

        vertices.push_back(vertex);
    }

    // Índices 
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Texturas
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Intentar cargar diffuse 
        aiString texPath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
        {
            std::string rawName = texPath.C_Str();
            std::string filename = extractFilename(rawName);

            if (!filename.empty())
            {
                std::string fullPath = directory + "/" + filename;
                std::cout << "  Diffuse: " << fullPath << std::endl;

                Texture tex;
                tex.id = TextureFromFile(fullPath.c_str());
                tex.type = "texture_diffuse";
                textures.push_back(tex);
            }
        }

        // Texturas para el powerbox
        if (directory.find("powerbox") != std::string::npos)
        {
            const char* extras[] = {
                "power_box_01_ao_2k.jpg",
                "power_box_01_rough_2k.jpg",
                "power_box_01_metal_2k.jpg",
                "power_box_01_arm_2k.jpg"
            };
            for (const char* file : extras)
            {
                std::string fullPath = directory + "/" + file;
                std::ifstream check(fullPath);
                if (check.good())
                {
                    check.close();
                    Texture tex;
                    tex.id = TextureFromFile(fullPath.c_str());
                    tex.type = "texture_extra";
                    textures.push_back(tex);
                }
            }
        }
    }

    return Mesh(vertices, indices, textures);
}