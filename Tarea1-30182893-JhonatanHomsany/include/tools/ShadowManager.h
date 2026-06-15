#pragma once
#include <glm/glm.hpp>
class Scene;
class Shader;
class Renderer;
enum class ShadowMode{
    NONE = 0,
    PLANAR = 1,
    SHADOW_MAPPING = 2,
    SHADOW_VOLUMES = 3
};
enum class ShadowMappingType{
    DIRECTIONAL = 0,
    SPOT = 1,
};
class ShadowManager {
public:
    static ShadowMode mode;
    static ShadowMappingType shadowMappingType;
    static float biasForShadowMapping;
    static bool useAdaptativeBias;
    static bool usePCF;
    static int pcfKernelRadius;
    static bool showOnlyShadows;
    static bool showDepthMap;
    static bool showShadowVolumes;
    static unsigned int depthMapFBO;
    static unsigned int depthMapTexture;
    static const unsigned int SHADOW_WIDTH = 2048;
    static const unsigned int SHADOW_HEIGHT = 2048;
    static void initShadowFBO();
    static void cleanupShadowFBO();
    static glm::mat4 getShadowProjectionMatrix(const glm::vec3& lightPos, float groundHeight);
    static void renderPlanarShadows(const Scene* scene, const Shader* flatShader, const Renderer* renderer, const glm::vec3& lightPos, float groundHeight, const glm::mat4& view, const glm::mat4& projection);
    static glm::mat4 renderShadowMap(const Scene* scene, const Shader* shadowDepthShader, const Renderer* renderer, const glm::vec3& lightPos);
    static void renderShadowVolumes(const Scene* scene, const Shader* flatShader, const Shader* volumeShader, const Renderer* renderer, const glm::vec3& lightPos, const glm::mat4& view, const glm::mat4& projection);
};
