#pragma once

#include <glm/glm.hpp>

bool isLookingAtObject(
    glm::vec3 cameraPos,
    glm::vec3 cameraFront,
    glm::vec3 objectPos,
    float maxDistance
);