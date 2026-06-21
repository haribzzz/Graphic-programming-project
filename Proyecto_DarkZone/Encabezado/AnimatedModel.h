#pragma once
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>
#include <string>
#include "Mesh.h"

// Info de un bone
struct BoneInfo {
    int     id;
    glm::mat4 offsetMatrix; // bind-pose inversa
};

// Un keyframe de posición / rotación / escala
struct KeyPosition { glm::vec3   value; double time; };
struct KeyRotation { glm::quat   value; double time; };
struct KeyScale { glm::vec3   value; double time; };

// Canal de animación para UN bone
struct BoneChannel {
    std::string name;
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale>    scales;
};

// Una animación completa
struct Animation {
    std::string              name;
    double                   duration;   // en ticks
    double                   ticksPerSec;
    std::vector<BoneChannel> channels;
};

class AnimatedModel {
public:
    AnimatedModel(const std::string& modelPath);

    // Carga una animación adicional desde otro FBX
    void LoadAnimation(const std::string& name,
        const std::string& path);

    // Cambia la animación activa (si ya está sonando la misma no reinicia)
    void PlayAnimation(const std::string& name);

    // Avanza el tiempo y recalcula boneMatrices
    void Update(float deltaTime);

    // Dibuja enviando boneMatrices al shader
    void Draw(GLuint shader);

    glm::mat4 modelMatrix = glm::mat4(1.0f);

private:
    // ---- datos de malla ----
    std::vector<Mesh>    meshes;
    std::string          directory;

    // ---- datos de skeleton ----
    std::map<std::string, BoneInfo> boneMap;  // nombre ? id + offset
    int                             boneCount = 0;
    glm::mat4 globalInverse;                  // inversa del root

    // ---- animaciones ----
    std::map<std::string, Animation> animations;
    std::string  currentAnim = "";
    double       currentTime = 0.0; // en ticks

    // ---- resultado final (va al shader) ----
    std::vector<glm::mat4> boneMatrices; // tamaño = boneCount

    // ---- carga interna ----
    void loadMeshes(const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    void extractBones(aiMesh* mesh);
    void loadAnimationFromScene(const std::string& name,
        const aiScene* scene);

    // ---- interpolación ----
    void computeBoneTransforms(const std::string& animName,
        double time,
        aiNode* node,
        const glm::mat4& parentTransform,
        const aiScene* scene);

    glm::vec3 interpolatePosition(const BoneChannel& ch, double t);
    glm::quat interpolateRotation(const BoneChannel& ch, double t);
    glm::vec3 interpolateScale(const BoneChannel& ch, double t);

    // ---- helpers ----
    static glm::mat4 toGLM(const aiMatrix4x4& m);
    static glm::vec3 toGLM(const aiVector3D& v);
    static glm::quat toGLM(const aiQuaternion& q);

    // Guardamos la escena principal para recorrer el árbol de nodos
    // (Assimp libera la escena al destruir el Importer, por eso guardamos
    //  una copia del árbol de nodos en nuestra propia estructura)
    struct NodeData {
        std::string        name;
        glm::mat4          transform;
        std::vector<NodeData> children;
    };
    NodeData rootNode;
    void copyNodeHierarchy(const aiNode* src, NodeData& dst);
    void computeBoneTransformsNode(const std::string& animName,
        double time,
        const NodeData& node,
        const glm::mat4& parentTransform);
};
