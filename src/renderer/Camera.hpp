#pragma once

#include <glm/mat4x4.hpp>

class Camera
{
public:
    void update(
        float mouseDeltaX,
        float mouseDeltaY,
        float wheelDelta,
        bool viewportHovered,
        bool dragging
    );

    glm::mat4 viewMatrix() const;

    glm::mat4 projectionMatrix(
        float aspect
    ) const;

private:
    float yaw_ = 0.0f;
    float pitch_ = 0.15f;
    float distance_ = 3.0f;
};