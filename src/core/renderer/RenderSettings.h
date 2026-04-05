#pragma once

#include <glm/glm.hpp>

namespace Metaphysics {

enum class RenderMode {
    Solid = 0,
    Wireframe = 1,
    SolidWireframe = 2
};

struct LightSettings {
    glm::vec3 position = glm::vec3(5.0f, 8.0f, 5.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.3f;
    float diffuseStrength = 1.0f;
    float specularStrength = 0.5f;
};

struct RenderSettings {
    LightSettings light;
    RenderMode renderMode = RenderMode::Solid;
    glm::vec4 clearColor = glm::vec4(0.15f, 0.15f, 0.18f, 1.0f);
    glm::vec3 wireColor = glm::vec3(1.0f, 0.5f, 0.0f);

    bool showGrid = true;
    bool showAxisX = true;
    bool showAxisY = true;
    bool showAxisZ = true;
    bool gridColorNegativeAxes = true;
    float gridSize = 10.0f;
    float gridSpacing = 1.0f;
};

} // namespace Metaphysics
