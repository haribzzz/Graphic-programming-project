#pragma once
#include "AnimatedModel.h"
#include <glm/glm.hpp>
#include <string>
#include <functional>

enum class EnemyState { IDLE, CHASE, FLEE };

class Enemy {
public:
    Enemy(const std::string& modelPath,
        const std::string& idlePath,
        const std::string& walkPath,
        const std::string& runPath);

    void Update(float deltaTime,
        glm::vec3 playerPos,
        glm::vec3 flashlightDir,
        glm::vec3 flashlightPos,
        bool      flashlightOn);
    void Draw(GLuint shader);

    // Tuneable
    float detectionRange = 10.0f;
    float chaseSpeed = 3.0f;
    float fleeSpeed = 7.0f;
    float flashAngleDeg = 25.0f; // cono de linterna
    glm::vec3 position = glm::vec3(0.0f);

    // FIX: escala configurable desde Main.cpp (antes hardcodeada a 0.01f)
    // Debe ajustarse en relación a MAZE_SCALE del laberinto.
    float modelScale = 0.018f;

    // FIX: corrección de eje — Mixamo->GLB vía Blender a veces exporta
    // con el armature recostado (X-up en vez de Y-up).
    // Si el enemigo sigue acostado, prueba 90.0f o -90.0f.
    float pitchCorrectionDeg = 90.0f;

    // Permite forzar log de diagnóstico una sola vez
    bool debugLoggedOnce = false;

    // FIX: callback de colisión inyectado desde Main.cpp.
    // Debe devolver true si la posición dada es válida (dentro del
    // laberinto / fuera de paredes). Si no se asigna, el enemigo
    // se mueve libremente (comportamiento anterior).
    std::function<bool(glm::vec3)> collisionCheck = nullptr;

private:
    AnimatedModel model;
    EnemyState    state = EnemyState::IDLE;
    float         yaw = 0.0f;

    void updateState(glm::vec3 playerPos, glm::vec3 flashDir,
        glm::vec3 flashPos, bool flashOn);
    bool isFlashlightHitting(glm::vec3 flashPos,
        glm::vec3 flashDir);
    void moveTowards(glm::vec3 target, float speed, float dt);
    void moveAwayFrom(glm::vec3 target, float speed, float dt);
    void applyTransform();
};