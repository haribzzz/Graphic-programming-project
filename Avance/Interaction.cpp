#include "Interaction.h"
#include <glm/glm.hpp>

bool isLookingAtObject(
    glm::vec3 cameraPos,
    glm::vec3 cameraFront,
    glm::vec3 objectPos,
    float maxDistance
)
{
    glm::vec3 direction =
        glm::normalize(objectPos - cameraPos);

    float dotProduct =
        glm::dot(cameraFront, direction);

    float distance =
        glm::distance(cameraPos, objectPos);


    return (
        dotProduct > 0.70f &&
        distance < maxDistance
        );
}