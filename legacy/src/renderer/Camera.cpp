#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

void Camera::update(
    float mouseDeltaX,
    float mouseDeltaY,
    float wheelDelta,
    bool viewportHovered,
    bool dragging
)
{
    if (!viewportHovered)
    {
        return;
    }

    if (dragging)
    {
        yaw_ -= mouseDeltaX * 0.008f;
        pitch_ -= mouseDeltaY * 0.008f;

        pitch_ = std::clamp(
            pitch_,
            -1.35f,
            1.35f
        );
    }

    distance_ -= wheelDelta * 0.22f;

    distance_ = std::clamp(
        distance_,
        1.3f,
        8.0f
    );
}

glm::mat4 Camera::viewMatrix() const
{
    const float cosPitch =
        std::cos(pitch_);

    const glm::vec3 position(
        distance_ *
            cosPitch *
            std::sin(yaw_),

        distance_ *
            std::sin(pitch_),

        distance_ *
            cosPitch *
            std::cos(yaw_)
    );

    return glm::lookAt(
        position,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

glm::mat4 Camera::projectionMatrix(
    float aspect
) const
{
    glm::mat4 projection =
        glm::perspective(
            glm::radians(45.0f),
            aspect,
            0.01f,
            100.0f
        );

    projection[1][1] *= -1.0f;

    return projection;
}