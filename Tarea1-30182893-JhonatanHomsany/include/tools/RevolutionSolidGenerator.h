#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Mesh.h"
struct ProfileSegment {
    bool isBezier = false;
    glm::vec2 p0 = glm::vec2(0.0f);
    glm::vec2 p1 = glm::vec2(0.0f);
    glm::vec2 p2 = glm::vec2(0.0f);
    glm::vec2 p3 = glm::vec2(0.0f);
};
class RevolutionSolidGenerator {
public:
    static std::vector<Vertex> generate(const std::vector<ProfileSegment>& segments, int radialSegments, int samplePointsPerSegment = 20);
    static void loadCylinderPreset(vector<ProfileSegment>& currentSegments);
    static void loadConePreset(vector<ProfileSegment>& currentSegments);
    static void loadSpherePreset(vector<ProfileSegment>& currentSegments);
};
