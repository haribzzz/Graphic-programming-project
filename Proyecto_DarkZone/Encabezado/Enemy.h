#pragma once
#include "AnimatedModel.h"
#include <glm/glm.hpp>
#include <string>
#include <functional>

// MODIFICADO: Añadido el estado PATROL
enum class EnemyState { IDLE, PATROL, CHASE, FLEE };

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
    float detectionRange = 25.0f;
    float patrolSpeed = 2.0f;      // Nueva velocidad para cuando da vueltas solo
    float chaseSpeed = 5.5f;
    float fleeSpeed = 8.5f;
    float flashAngleDeg = 25.0f;
    glm::vec3 position = glm::vec3(0.0f);

    float modelScale = 0.018f;
    float pitchCorrectionDeg = 90.0f;
    bool debugLoggedOnce = false;

    std::function<bool(glm::vec3)> collisionCheck = nullptr;

private:
    AnimatedModel model;
    EnemyState    state = EnemyState::PATROL; // Iniciamos en PATROL para que se mueva solo
    float         yaw = 0.0f;

    // NUEVAS VARIABLES PARA EL MOVIMIENTO AUTÓNOMO
    glm::vec3 patrolTarget = glm::vec3(0.0f);
    float patrolTimer = 0.0f;
    float huntTimer = 0.0f;
    bool isHuntingPlayer = false;

    void updateState(glm::vec3 playerPos, glm::vec3 flashDir, glm::vec3 flashPos, bool flashOn, float dt);
    bool isFlashlightHitting(glm::vec3 flashPos, glm::vec3 flashDir);
    void moveTowards(glm::vec3 target, float speed, float dt);
    void moveAwayFrom(glm::vec3 target, float speed, float dt);
    void generateNewPatrolTarget();
    void applyTransform();
};