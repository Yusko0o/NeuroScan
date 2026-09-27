#include "renderer/Camera.hpp"
#include <iostream>
#include <cmath>
#include <limits>
static bool finite(const glm::mat4& m) {
    for (int c=0;c<4;++c) for(int r=0;r<4;++r) if(!std::isfinite(m[c][r])) return false;
    return true;
}
int main() {
    Camera c;
    for(int v=0;v<5;++v) {
        c.setView(v);
        if(!finite(c.viewMatrix())) return 1;
    }
    c.update(1e6f,1e6f,1e6f,true,true);
    if(c.distance()<1.3f || c.distance()>12 || !finite(c.viewMatrix())) return 2;
    c.update(0,0,-1e6f,true,false);
    if(c.distance()!=12) return 3;
    const auto before=c.position();
    c.update(100,100,20,false,true);
    if(c.position()!=before) return 4;
    c.update(std::numeric_limits<float>::quiet_NaN(),0,0,true,true);
    if(!finite(c.viewMatrix()) || !finite(c.projectionMatrix(0))) return 5;
    auto p=c.projectionMatrix(1);
    auto near=p*glm::vec4(0,0,-0.01f,1), far=p*glm::vec4(0,0,-100,1);
    if(std::abs(near.z/near.w)>0.0001f || std::abs(far.z/far.w-1)>0.0001f || p[1][1]>=0) return 6;
    std::cout << "Camera orbit, bounds, presets, finite matrices and Vulkan depth range: PASS\n";
}
