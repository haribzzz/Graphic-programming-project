#include "AnimatedModel.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#ifndef aiProcess_PopulateArmatureData
#define aiProcess_PopulateArmatureData 0x200000
#endif

// ---------- helpers estáticos ----------
glm::mat4 AnimatedModel::toGLM(const aiMatrix4x4& m) {
    return glm::transpose(glm::make_mat4(&m.a1));
}
glm::vec3 AnimatedModel::toGLM(const aiVector3D& v) {
    return { v.x, v.y, v.z };
}
glm::quat AnimatedModel::toGLM(const aiQuaternion& q) {
    return { q.w, q.x, q.y, q.z };
}

// Normaliza nombres Mixamo — FBX usa "mixamorig:" y GLB de Blender puede usar "mixamorig_"
static std::string normName(const std::string& n) {
    if (n.find("mixamorig:") != std::string::npos) return n.substr(n.find("mixamorig:") + 10);
    if (n.find("mixamorig_") != std::string::npos) return n.substr(n.find("mixamorig_") + 10);
    return n;
}

// ---------- constructor ----------
AnimatedModel::AnimatedModel(const std::string& path) {
    Assimp::Importer imp;

    // FIX 1: sin aiProcess_FlipUVs (rompe índices en GLB)
    // FIX 2: aiProcess_PopulateArmatureData necesario para bones en GLTF
    const aiScene* scene = imp.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights |
        aiProcess_PopulateArmatureData);
    
    if (!scene || !scene->mRootNode) {
        std::cerr << "AnimatedModel error: " << imp.GetErrorString() << "\n";
        return;
    }

    std::cout << "AnimatedModel OK: " << path
        << "  meshes=" << scene->mNumMeshes
        << "  anims=" << scene->mNumAnimations << "\n";

    directory = path.substr(0, path.find_last_of('/'));
    globalInverse = glm::inverse(toGLM(scene->mRootNode->mTransformation));

    copyNodeHierarchy(scene->mRootNode, rootNode);
    loadMeshes(scene);

    boneMatrices.resize(boneCount, glm::mat4(1.0f));
    std::cout << "  bones=" << boneCount << "\n";

    if (scene->mNumAnimations > 0)
        loadAnimationFromScene("default", scene);
}

// ---------- carga de meshes ----------
void AnimatedModel::loadMeshes(const aiScene* scene) {
    std::function<void(aiNode*)> processNode = [&](aiNode* node) {
        for (unsigned i = 0; i < node->mNumMeshes; i++) {
            aiMesh* m = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(m, scene));
        }
        for (unsigned i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i]);
        };
    processNode(scene->mRootNode);
}

Mesh AnimatedModel::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    // Vértices
    for (unsigned i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.Position = toGLM(mesh->mVertices[i]);
        v.Normal = mesh->HasNormals() ? toGLM(mesh->mNormals[i]) : glm::vec3(0);
        v.TexCoords = mesh->mTextureCoords[0]
            ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
            : glm::vec2(0);
        vertices.push_back(v);
    }

    // Índices
    for (unsigned i = 0; i < mesh->mNumFaces; i++)
        for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    // Bones
    for (unsigned b = 0; b < mesh->mNumBones; b++) {
        aiBone* bone = mesh->mBones[b];
        // FIX 3: usar normName() en lugar del substr manual
        std::string bname = normName(bone->mName.C_Str());

        if (boneMap.find(bname) == boneMap.end()) {
            BoneInfo bi;
            bi.id = boneCount++;
            bi.offsetMatrix = toGLM(bone->mOffsetMatrix);
            boneMap[bname] = bi;
        }
        int boneID = boneMap[bname].id;

        for (unsigned w = 0; w < bone->mNumWeights; w++) {
            unsigned vIdx = bone->mWeights[w].mVertexId;
            float    wVal = bone->mWeights[w].mWeight;
            if (vIdx >= vertices.size()) continue; // seguridad
            for (int s = 0; s < MAX_BONE_INFLUENCE; s++) {
                if (vertices[vIdx].boneIDs[s] == -1) {
                    vertices[vIdx].boneIDs[s] = boneID;
                    vertices[vIdx].weights[s] = wVal;
                    break;
                }
            }
        }
    }

    // Textura: GLB guarda texturas embebidas como "*0", "*1", etc.
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        aiString texPath;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::string texStr = texPath.C_Str();
            GLuint texID = 0;

            if (!texStr.empty() && texStr[0] == '*') {
                // Textura embebida dentro del GLB
                int idx = std::stoi(texStr.substr(1));
                const aiTexture* embTex = scene->mTextures[idx];

                if (embTex->mHeight == 0) { // comprimida (jpg/png)
                    int w, h, ch;
                    unsigned char* data = stbi_load_from_memory(
                        (unsigned char*)embTex->pcData,
                        embTex->mWidth, &w, &h, &ch, 0);
                    if (data) {
                        glGenTextures(1, &texID);
                        glBindTexture(GL_TEXTURE_2D, texID);
                        GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
                        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
                        glGenerateMipmap(GL_TEXTURE_2D);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        stbi_image_free(data);
                    }
                }
            }
            else {
                // Textura externa (archivo en disco)
                extern GLuint TextureFromFile(const char*);
                size_t p = texStr.find_last_of("/\\");
                if (p != std::string::npos) texStr = texStr.substr(p + 1);
                texID = TextureFromFile((directory + "/" + texStr).c_str());
            }

            if (texID != 0) {
                Texture t; t.id = texID; t.type = "texture_diffuse";
                textures.push_back(t);
            }
        }
    }

    return Mesh(vertices, indices, textures);
}

// ---------- jerarquía de nodos ----------
void AnimatedModel::copyNodeHierarchy(const aiNode* src, NodeData& dst) {
    // FIX 4: usar normName() aquí también
    dst.name = normName(src->mName.C_Str());
    dst.transform = toGLM(src->mTransformation);
    dst.children.resize(src->mNumChildren);
    for (unsigned i = 0; i < src->mNumChildren; i++)
        copyNodeHierarchy(src->mChildren[i], dst.children[i]);
}

// ---------- cargar animación externa ----------
void AnimatedModel::LoadAnimation(const std::string& name,
    const std::string& path) {
    Assimp::Importer imp;

    // FIX 5: mismos flags que el constructor
    const aiScene* scene = imp.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_LimitBoneWeights |
        aiProcess_PopulateArmatureData);

    if (!scene || scene->mNumAnimations == 0) {
        std::cerr << "LoadAnimation error (" << name << "): "
            << imp.GetErrorString() << "\n";
        return;
    }
    loadAnimationFromScene(name, scene);
    std::cout << "Animacion cargada: " << name << "\n";
}

void AnimatedModel::loadAnimationFromScene(const std::string& name,
    const aiScene* scene) {
    aiAnimation* anim = scene->mAnimations[0];
    Animation a;
    a.name = name;
    a.duration = anim->mDuration;
    a.ticksPerSec = anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0;

    for (unsigned c = 0; c < anim->mNumChannels; c++) {
        aiNodeAnim* ch = anim->mChannels[c];
        BoneChannel bc;
        // FIX 6: normName() en los canales de animación
        bc.name = normName(ch->mNodeName.C_Str());

        for (unsigned k = 0; k < ch->mNumPositionKeys; k++)
            bc.positions.push_back({ toGLM(ch->mPositionKeys[k].mValue),
                                     ch->mPositionKeys[k].mTime });
        for (unsigned k = 0; k < ch->mNumRotationKeys; k++)
            bc.rotations.push_back({ toGLM(ch->mRotationKeys[k].mValue),
                                     ch->mRotationKeys[k].mTime });
        for (unsigned k = 0; k < ch->mNumScalingKeys; k++)
            bc.scales.push_back({ toGLM(ch->mScalingKeys[k].mValue),
                                  ch->mScalingKeys[k].mTime });

        a.channels.push_back(bc);
    }
    animations[name] = a;
}

// ---------- control de reproducción ----------
void AnimatedModel::PlayAnimation(const std::string& name) {
    if (currentAnim == name) return;
    if (animations.find(name) == animations.end()) return;
    currentAnim = name;
    currentTime = 0.0;
}

void AnimatedModel::Update(float dt) {
    if (currentAnim.empty() || animations.find(currentAnim) == animations.end())
        return;

    const Animation& anim = animations.at(currentAnim);
    currentTime += anim.ticksPerSec * dt;
    currentTime = fmod(currentTime, anim.duration);

    boneMatrices.assign(boneCount, glm::mat4(1.0f));
    computeBoneTransformsNode(currentAnim, currentTime, rootNode, globalInverse);
}

// ---------- interpolación ----------
glm::vec3 AnimatedModel::interpolatePosition(const BoneChannel& ch, double t) {
    if (ch.positions.size() == 1) return ch.positions[0].value;
    for (size_t i = 0; i < ch.positions.size() - 1; i++) {
        if (t < ch.positions[i + 1].time) {
            double seg = ch.positions[i + 1].time - ch.positions[i].time;
            float  f = (float)((t - ch.positions[i].time) / seg);
            return glm::mix(ch.positions[i].value, ch.positions[i + 1].value, f);
        }
    }
    return ch.positions.back().value;
}

glm::quat AnimatedModel::interpolateRotation(const BoneChannel& ch, double t) {
    if (ch.rotations.size() == 1) return ch.rotations[0].value;
    for (size_t i = 0; i < ch.rotations.size() - 1; i++) {
        if (t < ch.rotations[i + 1].time) {
            double seg = ch.rotations[i + 1].time - ch.rotations[i].time;
            float  f = (float)((t - ch.rotations[i].time) / seg);
            return glm::slerp(ch.rotations[i].value, ch.rotations[i + 1].value, f);
        }
    }
    return ch.rotations.back().value;
}

glm::vec3 AnimatedModel::interpolateScale(const BoneChannel& ch, double t) {
    if (ch.scales.size() == 1) return ch.scales[0].value;
    for (size_t i = 0; i < ch.scales.size() - 1; i++) {
        if (t < ch.scales[i + 1].time) {
            double seg = ch.scales[i + 1].time - ch.scales[i].time;
            float  f = (float)((t - ch.scales[i].time) / seg);
            return glm::mix(ch.scales[i].value, ch.scales[i + 1].value, f);
        }
    }
    return ch.scales.back().value;
}

// ---------- recorrido del árbol ----------
void AnimatedModel::computeBoneTransformsNode(
    const std::string& animName, double time,
    const NodeData& node, const glm::mat4& parentTransform)
{
    glm::mat4 nodeTransform = node.transform;

    const Animation& anim = animations.at(animName);

    for (const BoneChannel& ch : anim.channels) {
        if (ch.name == node.name) {
            glm::mat4 T = glm::translate(glm::mat4(1.0f), interpolatePosition(ch, time));
            glm::mat4 R = glm::toMat4(interpolateRotation(ch, time));
            glm::mat4 S = glm::scale(glm::mat4(1.0f), interpolateScale(ch, time));
            nodeTransform = T * R * S;
            break;
        }
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    auto it = boneMap.find(node.name);
    if (it != boneMap.end()) {
        int id = it->second.id;
        boneMatrices[id] = globalTransform * it->second.offsetMatrix;
    }

    for (const NodeData& child : node.children)
        computeBoneTransformsNode(animName, time, child, globalTransform);
}

// ---------- Draw ----------
void AnimatedModel::Draw(GLuint shader) {
    bool animated = !currentAnim.empty() && boneCount > 0;
    glUniform1i(glGetUniformLocation(shader, "animated"), animated ? 1 : 0);

    if (animated) {
        for (int i = 0; i < boneCount; i++) {
            std::string u = "boneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(
                glGetUniformLocation(shader, u.c_str()),
                1, GL_FALSE, glm::value_ptr(boneMatrices[i]));
        }
    }

    glUniformMatrix4fv(
        glGetUniformLocation(shader, "model"),
        1, GL_FALSE, glm::value_ptr(modelMatrix));

    for (auto& m : meshes)
        m.Draw(shader);
}