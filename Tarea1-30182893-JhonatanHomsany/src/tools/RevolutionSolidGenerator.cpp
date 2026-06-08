#include "../../include/tools/RevolutionSolidGenerator.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
std::vector<Vertex> RevolutionSolidGenerator::generate(const std::vector<ProfileSegment>& segments, int radialSegments, int samplePointsPerSegment) {
    std::vector<Vertex> result;
    if (segments.empty() || radialSegments < 3) return result;
    std::vector<glm::vec2> profilePoints;
    for (const auto& seg : segments) {
        if (seg.isBezier) {
            for (int step = 0; step <= samplePointsPerSegment; ++step) {
                float t = (float)step / samplePointsPerSegment;
                if (step == 0 && !profilePoints.empty()) continue;
                float u = 1.0f - t;
                glm::vec2 p = u*u*u * seg.p0 + 
                              3.0f * u*u * t * seg.p1 + 
                              3.0f * u * t*t * seg.p2 + 
                              t*t*t * seg.p3;
                profilePoints.push_back(p);
            }
        } else {
            for (int step = 0; step <= samplePointsPerSegment; ++step) {
                float t = (float)step / samplePointsPerSegment;
                if (step == 0 && !profilePoints.empty()) continue; 
                glm::vec2 p = (1.0f - t) * seg.p0 + t * seg.p1;
                profilePoints.push_back(p);
            }
        }
    }
    size_t M = profilePoints.size();
    if (M < 2) return result;
    std::vector<glm::vec2> tangents(M);
    tangents[0] = glm::normalize(profilePoints[1] - profilePoints[0]);
    for (size_t i = 1; i < M - 1; ++i) {
        glm::vec2 diff = profilePoints[i+1] - profilePoints[i-1];
        float len = glm::length(diff);
        if (len > 0.0001f) {
            tangents[i] = diff / len;
        } else {
            tangents[i] = tangents[i-1];
        }
    }
    glm::vec2 lastDiff = profilePoints[M-1] - profilePoints[M-2];
    if (glm::length(lastDiff) > 0.0001f) {
        tangents[M-1] = glm::normalize(lastDiff);
    } else {
        tangents[M-1] = tangents[M-2];
    }
    auto getVertex = [&](size_t i, int j) -> Vertex {
        float theta = 2.0f * glm::pi<float>() * j / radialSegments;
        float cosT = std::cos(theta);
        float sinT = std::sin(theta);
        glm::vec3 pos(profilePoints[i].x * cosT, profilePoints[i].y, profilePoints[i].x * sinT);
        float dx = tangents[i].x;
        float dy = tangents[i].y;
        glm::vec3 norm(dy * cosT, -dx, dy * sinT);
        if (glm::length(norm) > 0.0001f) {
            norm = glm::normalize(norm);
        } else {
            norm = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        Vertex v;
        v.position = pos;
        v.normal = norm;
        return v;
    };
    for (size_t i = 0; i < M - 1; ++i) {
        for (int j = 0; j < radialSegments; ++j) {
            int nextJ = j + 1;
            Vertex v00 = getVertex(i, j);
            Vertex v10 = getVertex(i + 1, j);
            Vertex v01 = getVertex(i, nextJ);
            Vertex v11 = getVertex(i + 1, nextJ);
            result.push_back(v00);
            result.push_back(v10);
            result.push_back(v11);
            result.push_back(v00);
            result.push_back(v11);
            result.push_back(v01);
        }
    }
    return result;
}
void RevolutionSolidGenerator::loadCylinderPreset(vector<ProfileSegment>& currentSegments) {
    currentSegments.clear();
    ProfileSegment base;
    base.isBezier = false;
    base.p0 = glm::vec2(0.0f, -1.0f);
    base.p1 = glm::vec2(0.8f, -1.0f);
    currentSegments.push_back(base);
    ProfileSegment wall;
    wall.isBezier = false;
    wall.p0 = glm::vec2(0.8f, -1.0f);
    wall.p1 = glm::vec2(0.8f, 1.0f);
    currentSegments.push_back(wall);
    ProfileSegment top;
    top.isBezier = false;
    top.p0 = glm::vec2(0.8f, 1.0f);
    top.p1 = glm::vec2(0.0f, 1.0f);
    currentSegments.push_back(top);
}
void RevolutionSolidGenerator::loadConePreset(vector<ProfileSegment>& currentSegments) {
    currentSegments.clear();
    ProfileSegment base;
    base.isBezier = false;
    base.p0 = glm::vec2(0.0f, -1.0f);
    base.p1 = glm::vec2(0.8f, -1.0f);
    currentSegments.push_back(base);
    ProfileSegment side;
    side.isBezier = false;
    side.p0 = glm::vec2(0.8f, -1.0f);
    side.p1 = glm::vec2(0.0f, 1.0f);
    currentSegments.push_back(side);
}
void RevolutionSolidGenerator::loadSpherePreset(vector<ProfileSegment>& currentSegments) {
    currentSegments.clear();
    const float KAPPA = 0.5522847498f;
    const float RADIUS = 1.0f;
    ProfileSegment bottomCurve;
    bottomCurve.isBezier = true;
    bottomCurve.p0 = glm::vec2(0.0f, -RADIUS);
    bottomCurve.p1 = glm::vec2(RADIUS * KAPPA, -RADIUS);
    bottomCurve.p2 = glm::vec2(RADIUS, -RADIUS * KAPPA);
    bottomCurve.p3 = glm::vec2(RADIUS, 0.0f);
    currentSegments.push_back(bottomCurve);
    ProfileSegment topCurve;
    topCurve.isBezier = true;
    topCurve.p0 = glm::vec2(RADIUS, 0.0f);
    topCurve.p1 = glm::vec2(RADIUS, RADIUS * KAPPA);
    topCurve.p2 = glm::vec2(RADIUS * KAPPA, RADIUS);
    topCurve.p3 = glm::vec2(0.0f, RADIUS);
    currentSegments.push_back(topCurve);
}