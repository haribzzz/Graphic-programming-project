#include "Enemy.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <cmath>
#include <iostream>
#include <cstdlib> // Para generar números aleatorios

Enemy::Enemy(const std::string& modelPath,
    const std::string& idlePath,
    const std::string& walkPath,
    const std::string& runPath)
    : model(modelPath)
{
    model.LoadAnimation("idle", idlePath);
    model.LoadAnimation("walk", walkPath);
    model.LoadAnimation("run", runPath);

    model.PlayAnimation("idle");

    // Inicializar el primer punto de patrulla
    generateNewPatrolTarget();
}

void Enemy::Update(float dt,
    glm::vec3 playerPos,
    glm::vec3 flashDir,
    glm::vec3 flashPos,
    bool flashOn)
{
    EnemyState previousState = state;

    // Pasamos dt a la máquina de estados para controlar los temporizadores
    updateState(playerPos, flashDir, flashPos, flashOn, dt);

    switch (state) {
    case EnemyState::IDLE:
        model.PlayAnimation("idle");
        break;
    case EnemyState::PATROL:
        model.PlayAnimation("walk");
        // Se mueve solo hacia el punto aleatorio de patrulla
        moveTowards(patrolTarget, patrolSpeed, dt);
        break;
    case EnemyState::CHASE:
        model.PlayAnimation("walk");
        // Te persigue directamente a tu ubicación
        moveTowards(playerPos, chaseSpeed, dt);
        break;
    case EnemyState::FLEE:
        model.PlayAnimation("run");
        moveAwayFrom(playerPos, fleeSpeed, dt);
        break;
    }

    if (state != previousState) {
        const char* stateName =
            (state == EnemyState::IDLE) ? "IDLE (idle)" :
            (state == EnemyState::PATROL) ? "PATROL (vueltas solo)" :
            (state == EnemyState::CHASE) ? "CHASE (persiguiendo)" :
            "FLEE (run)";
        std::cout << "[Enemy] Cambio de estado -> " << stateName << std::endl;
    }

    applyTransform();
    model.Update(dt);
}

void Enemy::Draw(GLuint shader) {
    model.Draw(shader);
}

// ---------- NUEVA LÓGICA DE ESTADOS CON IA TOTALMENTE AUTÓNOMA ----------
void Enemy::updateState(glm::vec3 playerPos, glm::vec3 flashDir, glm::vec3 flashPos, bool flashOn, float dt)
{
    // Prioridad 1: Linterna encendida apuntándole -> Huir siempre
    if (flashOn && isFlashlightHitting(flashPos, flashDir)) {
        state = EnemyState::FLEE;
        isHuntingPlayer = false;
        return;
    }

    // CONTROL DEL TEMPORIZADOR DE "CAZA" (Saber tu ubicación siempre, pero actuar a veces)
    huntTimer += dt;
    if (!isHuntingPlayer && huntTimer > 15.0f) { // Cada 15 segundos de patrulla pacífica...
        if ((rand() % 100) < 40) { // 40% de probabilidades de que decida ir a buscarte
            isHuntingPlayer = true;
            std::cout << "[Enemy] IA: He detectado tu rastro... yendo a tu posición." << std::endl;
        }
        huntTimer = 0.0f;
    }

    // Prioridad 2: Persecución activa
    float distToPlayer = glm::distance(position, playerPos);

    // Te persigue si entras en su rango de visión clásico O si la IA activó el modo de caza global
    if (distToPlayer < detectionRange || isHuntingPlayer) {
        state = EnemyState::CHASE;

        // Si ya llegó muy cerca en modo caza, desactiva el modo global para volver a patrullar si te pierde
        if (distToPlayer < 4.0f) {
            isHuntingPlayer = false;
        }
        return;
    }

    // Prioridad 3: Movimiento autónomo por defecto (PATROL)
    state = EnemyState::PATROL;

    // Control del destino de patrulla: cambiar si pasa mucho tiempo o si llegó al destino
    patrolTimer += dt;
    if (glm::distance(position, patrolTarget) < 1.5f || patrolTimer > 8.0f) {
        generateNewPatrolTarget();
    }
}

// Genera una dirección aleatoria en base a donde se encuentra actualmente
void Enemy::generateNewPatrolTarget() {
    patrolTimer = 0.0f;

    // Genera un desplazamiento aleatorio en un radio de entre 10 y 25 unidades en X y Z
    float randomX = ((float)(rand() % 50) - 25.0f);
    float randomZ = ((float)(rand() % 50) - 25.0f);

    // El nuevo objetivo será su posición actual más el desfase aleatorio
    patrolTarget = position + glm::vec3(randomX, 0.0f, randomZ);
}

bool Enemy::isFlashlightHitting(glm::vec3 flashPos, glm::vec3 flashDir)
{
    glm::vec3 toEnemy = position - flashPos;
    float dist = glm::length(toEnemy);
    if (dist < 0.001f) return false;
    glm::vec3 toEnemyN = toEnemy / dist;
    float dot = glm::dot(glm::normalize(flashDir), toEnemyN);
    float threshold = glm::cos(glm::radians(flashAngleDeg));
    return dot > threshold;
}

// ---------- MOVIMIENTO CON CONTROL DE DESLIZAMIENTO ----------
void Enemy::moveTowards(glm::vec3 target, float speed, float dt) {
    glm::vec3 dir = target - position;
    dir.y = 0.0f;
    if (glm::length2(dir) < 0.0001f) return;
    dir = glm::normalize(dir);

    glm::vec3 velocity = dir * speed * dt;
    glm::vec3 nextPos = position + velocity;

    if (collisionCheck) {
        if (collisionCheck(nextPos)) {
            position = nextPos;
        }
        else {
            // Deslizamiento en paredes para no quedarse atascado en el laboratorio solo
            glm::vec3 testX = position + glm::vec3(velocity.x, 0.0f, 0.0f);
            glm::vec3 testZ = position + glm::vec3(0.0f, 0.0f, velocity.z);

            if (collisionCheck(testX)) {
                position = testX;
            }
            else if (collisionCheck(testZ)) {
                position = testZ;
            }
            else {
                // Si se encuentra en una esquina cerrada patrullando solo, forzar cambio de rumbo
                if (state == EnemyState::PATROL) {
                    generateNewPatrolTarget();
                }
            }
        }
    }
    else {
        position = nextPos;
    }

    yaw = glm::degrees(std::atan2(dir.x, dir.z));
}

void Enemy::moveAwayFrom(glm::vec3 target, float speed, float dt) {
    glm::vec3 dir = position - target;
    dir.y = 0.0f;
    if (glm::length2(dir) < 0.0001f) return;
    dir = glm::normalize(dir);

    glm::vec3 velocity = dir * speed * dt;
    glm::vec3 nextPos = position + velocity;

    if (collisionCheck) {
        if (collisionCheck(nextPos)) {
            position = nextPos;
        }
        else {
            glm::vec3 testX = position + glm::vec3(velocity.x, 0.0f, 0.0f);
            glm::vec3 testZ = position + glm::vec3(0.0f, 0.0f, velocity.z);

            if (collisionCheck(testX)) {
                position = testX;
            }
            else if (collisionCheck(testZ)) {
                position = testZ;
            }
        }
    }
    else {
        position = nextPos;
    }

    yaw = glm::degrees(std::atan2(dir.x, dir.z));
}

void Enemy::applyTransform() {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, glm::radians(yaw), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(pitchCorrectionDeg), glm::vec3(1, 0, 0));
    m = glm::scale(m, glm::vec3(modelScale));
    model.modelMatrix = m;

    if (!debugLoggedOnce) {
        debugLoggedOnce = true;
    }
}