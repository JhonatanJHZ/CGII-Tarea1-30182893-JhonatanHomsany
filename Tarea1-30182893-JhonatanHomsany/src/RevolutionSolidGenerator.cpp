#include "../include/RevolutionSolidGenerator.h"
#include <glm/gtc/constants.hpp>
#include <cmath>

std::vector<Vertex> RevolutionSolidGenerator::generate(const std::vector<ProfileSegment>& segments, int radialSegments, int samplePointsPerSegment) {
    std::vector<Vertex> result;
    if (segments.empty() || radialSegments < 3) return result;

    // 1. Muestrear los segmentos del perfil
    std::vector<glm::vec2> profilePoints;
    for (const auto& seg : segments) {
        if (seg.isBezier) {
            for (int step = 0; step <= samplePointsPerSegment; ++step) {
                float t = (float)step / samplePointsPerSegment;
                if (step == 0 && !profilePoints.empty()) continue; // Evitar duplicar punto de unión
                
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
                if (step == 0 && !profilePoints.empty()) continue; // Evitar duplicar punto de unión
                
                glm::vec2 p = (1.0f - t) * seg.p0 + t * seg.p1;
                profilePoints.push_back(p);
            }
        }
    }

    size_t M = profilePoints.size();
    if (M < 2) return result;

    // 2. Precalcular tangentes del perfil 2D para normales suaves
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

    // Función auxiliar para generar un vértice dadas las coordenadas del perfil (i) y radial (j)
    auto getVertex = [&](size_t i, int j) -> Vertex {
        float theta = 2.0f * glm::pi<float>() * j / radialSegments;
        float cosT = std::cos(theta);
        float sinT = std::sin(theta);

        glm::vec3 pos(profilePoints[i].x * cosT, profilePoints[i].y, profilePoints[i].x * sinT);

        float dx = tangents[i].x;
        float dy = tangents[i].y;
        
        // El vector normal se orienta hacia afuera del sólido de revolución
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

    // 3. Construir los triángulos de la malla
    for (size_t i = 0; i < M - 1; ++i) {
        for (int j = 0; j < radialSegments; ++j) {
            int nextJ = j + 1;

            Vertex v00 = getVertex(i, j);
            Vertex v10 = getVertex(i + 1, j);
            Vertex v01 = getVertex(i, nextJ);
            Vertex v11 = getVertex(i + 1, nextJ);

            // Triángulo 1 (v00 -> v10 -> v11)
            result.push_back(v00);
            result.push_back(v10);
            result.push_back(v11);

            // Triángulo 2 (v00 -> v11 -> v01)
            result.push_back(v00);
            result.push_back(v11);
            result.push_back(v01);
        }
    }

    return result;
}
