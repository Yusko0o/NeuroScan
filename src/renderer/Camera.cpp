#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <algorithm>
#include <cmath>
void Camera::update(float dx, float dy, float wheel, bool hovered, bool dragging) {
    if (!hovered || !std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(wheel)) return;
    if (dragging) {
        yaw_ = std::remainder(yaw_ - dx * 0.008f, 6.2831853f);
        pitch_ = std::clamp(pitch_ - dy * 0.008f, -1.55f, 1.55f);
    }
    distance_ = std::clamp(distance_ * std::exp(-wheel * 0.12f), 1.3f, 12.0f);
}
void Camera::setView(int view) {
    *this = Camera{};
    switch (view) {
    case 1: yaw_ = 0; pitch_ = 0; break;
    case 2: yaw_ = 1.5707963f; pitch_ = 0; break;
    case 3: yaw_ = 0; pitch_ = 1.55f; break;
    case 4: yaw_ = 3.1415927f; pitch_ = 0; break;
    default: break;
    }
}
glm::vec3 Camera::position() const {
    return distance_ * glm::vec3(std::cos(pitch_) * std::sin(yaw_), std::sin(pitch_),
                                 std::cos(pitch_) * std::cos(yaw_));
}
glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position(), glm::vec3(0), glm::vec3(0, 1, 0));
}
glm::mat4 Camera::projectionMatrix(float aspect) const {
    if (!std::isfinite(aspect) || aspect <= 0) aspect = 1;
    auto p = glm::perspectiveRH_ZO(glm::radians(45.0f), aspect, 0.01f, 100.0f);
    p[1][1] *= -1;
    return p;
}
