#include "Enemy.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <cmath>

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
}

void Enemy::Update(float dt,
    glm::vec3 playerPos,
    glm::vec3 flashDir,
    glm::vec3 flashPos,
    bool flashOn)
{
    updateState(playerPos, flashDir, flashPos, flashOn);

    switch (state) {
    case EnemyState::IDLE:
        model.PlayAnimation("idle");
        break;

    case EnemyState::CHASE:
        model.PlayAnimation("walk");
        moveTowards(playerPos, chaseSpeed, dt);
        break;

    case EnemyState::FLEE:
        model.PlayAnimation("run");
        moveAwayFrom(playerPos, fleeSpeed, dt);
        break;
    }

    applyTransform();
    model.Update(dt);
}

void Enemy::Draw(GLuint shader) {
    model.Draw(shader);
}

// ---------- lógica de estado ----------
void Enemy::updateState(glm::vec3 playerPos, glm::vec3 flashDir,
    glm::vec3 flashPos, bool flashOn)
{
    // Prioridad 1: linterna apuntando ? huir siempre
    if (flashOn && isFlashlightHitting(flashPos, flashDir)) {
        state = EnemyState::FLEE;
        return;
    }
    // Prioridad 2: jugador dentro del rango ? perseguir
    float dist = glm::distance(position, playerPos);
    if (dist < detectionRange) {
        state = EnemyState::CHASE;
        return;
    }
    state = EnemyState::IDLE;
}

bool Enemy::isFlashlightHitting(glm::vec3 flashPos,
    glm::vec3 flashDir)
{
    glm::vec3 toEnemy = position - flashPos;
    float dist = glm::length(toEnemy);
    if (dist < 0.001f) return false;

    glm::vec3 toEnemyN = toEnemy / dist;
    float dot = glm::dot(glm::normalize(flashDir), toEnemyN);
    float threshold = glm::cos(glm::radians(flashAngleDeg));
    return dot > threshold;
}

// ---------- movimiento ----------
void Enemy::moveTowards(glm::vec3 target, float speed, float dt) {
    glm::vec3 dir = target - position;
    dir.y = 0.0f;
    if (glm::length2(dir) < 0.0001f) return;
    dir = glm::normalize(dir);
    position += dir * speed * dt;
    yaw = glm::degrees(std::atan2(dir.x, dir.z));
}

void Enemy::moveAwayFrom(glm::vec3 target, float speed, float dt) {
    glm::vec3 dir = position - target;
    dir.y = 0.0f;
    if (glm::length2(dir) < 0.0001f) return;
    dir = glm::normalize(dir);
    position += dir * speed * dt;
    yaw = glm::degrees(std::atan2(dir.x, dir.z));
}

void Enemy::applyTransform() {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, glm::radians(yaw), glm::vec3(0, 1, 0));
    m = glm::scale(m, glm::vec3(0.01f)); // ajusta escala según tu escena
    model.modelMatrix = m;
}