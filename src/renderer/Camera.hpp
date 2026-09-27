#pragma once
#include <glm/glm.hpp>
class Camera {
public:
    void update(float dx, float dy, float wheel, bool hovered, bool dragging);
    void setView(int view);
    glm::vec3 position() const;
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspect) const;
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }
    float distance() const { return distance_; }
private:
    float yaw_ = 0.0f;
    float pitch_ = 0.15f;
    float distance_ = 3.6f;
};
