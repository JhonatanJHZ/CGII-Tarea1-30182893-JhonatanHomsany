#include "../include/tools/ShadowManager.h"
#include "../include/Scene.h"
#include "../include/Shader.h"
#include "../include/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../include/Mesh.h"
#include "../include/tools/GLTFManager.h"
#include <glm/gtc/type_ptr.hpp>

ShadowMode ShadowManager::mode = ShadowMode::PLANAR;
ShadowMappingType ShadowManager::shadowMappingType = ShadowMappingType::DIRECTIONAL;
float ShadowManager::biasForShadowMapping = 0.0001f;
bool ShadowManager::useAdaptativeBias = false;
bool ShadowManager::usePCF = true;
int ShadowManager::pcfKernelRadius = 1;
bool ShadowManager::showOnlyShadows = false;
bool ShadowManager::showDepthMap = false;
bool ShadowManager::showShadowVolumes = false;
unsigned int ShadowManager::depthMapFBO = 0;
unsigned int ShadowManager::depthMapTexture = 0;
#include <iostream>
void ShadowManager::initShadowFBO() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: El Framebuffer de sombras no está completo." << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void ShadowManager::cleanupShadowFBO() {
    if (depthMapFBO != 0) {
        glDeleteFramebuffers(1, &depthMapFBO);
        depthMapFBO = 0;
    }
    if (depthMapTexture != 0) {
        glDeleteTextures(1, &depthMapTexture);
        depthMapTexture = 0;
    }
}
glm::mat4 ShadowManager::getShadowProjectionMatrix(const glm::vec3& lightPos, float groundHeight) {
    glm::mat4 shadowMat(1.0f);
    float lx = lightPos.x;
    float ly = lightPos.y;
    float lz = lightPos.z;
    shadowMat[0][0] = ly - groundHeight;
    shadowMat[0][1] = 0.0f;
    shadowMat[0][2] = 0.0f;
    shadowMat[0][3] = 0.0f;
    shadowMat[1][0] = -lx;
    shadowMat[1][1] = -groundHeight;
    shadowMat[1][2] = -lz;
    shadowMat[1][3] = -1.0f;
    shadowMat[2][0] = 0.0f;
    shadowMat[2][1] = 0.0f;
    shadowMat[2][2] = ly - groundHeight;
    shadowMat[2][3] = 0.0f;
    shadowMat[3][0] = lx * groundHeight;
    shadowMat[3][1] = ly * groundHeight;
    shadowMat[3][2] = lz * groundHeight;
    shadowMat[3][3] = ly;
    return shadowMat;
}
void ShadowManager::renderPlanarShadows(
    const Scene* scene,
    const Shader* flatShader,
    const Renderer* renderer,
    const glm::vec3& lightPos,
    float groundHeight,
    const glm::mat4& view,
    const glm::mat4& projection
) {
    if (!scene || !flatShader || !renderer) return;
    glm::mat4 shadowMatrix = getShadowProjectionMatrix(lightPos, groundHeight);
    glUseProgram(flatShader->ID);
    glUniform4f(glGetUniformLocation(flatShader->ID, "objectColor"), 0.1f, 0.1f, 0.1f, 0.5f);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);
    for (const auto& obj : scene->objects) {
        if (!obj.meshPointer) continue;
        if (obj.name == "Piso" || obj.name == "Techo" || 
            obj.name == "Pared Izquierda" || obj.name == "Pared Derecha" || 
            obj.name == "Pared del Frente" || obj.name == "Pared Trasera") {
            continue;
        }
        glm::mat4 shadowModelMatrix = shadowMatrix * obj.getModelMatrix();
        if (obj.type == MeshType::GLTF) {
            renderer->render(static_cast<const GLTFManager*>(obj.meshPointer), flatShader, shadowModelMatrix, view, projection);
        } else if (obj.type == MeshType::REVOLUTION_SOLID) {
            renderer->render(static_cast<const Mesh*>(obj.meshPointer), flatShader, shadowModelMatrix, view, projection);
        }
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
}
glm::mat4 ShadowManager::renderShadowMap(const Scene* scene, const Shader* shadowDepthShader, const Renderer* renderer, const glm::vec3& lightPos){
    if (!scene || !shadowDepthShader || !renderer) return glm::mat4(1.0f);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(shadowDepthShader->ID);
    glm::mat4 lightProjection;
    glm::mat4 lightView;
    float near_plane = 1.0f, far_plane = 25.0f;
    if (shadowMappingType == ShadowMappingType::DIRECTIONAL) {
        float size = 12.0f;
        lightProjection = glm::ortho(-size, size, -size, size, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f, -7.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
        lightProjection = glm::perspective(glm::radians(110.0f), 1.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f, -7.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    glUniformMatrix4fv(glGetUniformLocation(shadowDepthShader->ID, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    for (const auto& obj : scene->objects) {
        if (!obj.meshPointer) continue;
        if (obj.name == "Piso" || obj.name == "Techo" || 
            obj.name == "Pared Izquierda" || obj.name == "Pared Derecha" || 
            obj.name == "Pared del Frente" || obj.name == "Pared Trasera") {
            continue;
        }
        glm::mat4 shadowModelMatrix = obj.getModelMatrix();
        if (obj.type == MeshType::GLTF) {
            renderer->render(static_cast<const GLTFManager*>(obj.meshPointer), shadowDepthShader, shadowModelMatrix, glm::mat4(1.0f), glm::mat4(1.0f));
        } else if (obj.type == MeshType::REVOLUTION_SOLID) {
            renderer->render(static_cast<const Mesh*>(obj.meshPointer), shadowDepthShader, shadowModelMatrix, glm::mat4(1.0f), glm::mat4(1.0f));
        }
    }
    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return lightSpaceMatrix;
}

void ShadowManager::renderShadowVolumes(const Scene* scene, const Shader* flatShader, const Shader* volumeShader, const Renderer* renderer, const glm::vec3& lightPos, const glm::mat4& view, const glm::mat4& projection) {
    if (!scene || !flatShader || !volumeShader || !renderer) return;

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF); 
    glClear(GL_STENCIL_BUFFER_BIT); 

    glStencilFunc(GL_ALWAYS, 0, 0xFF); 
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
    glDepthMask(GL_FALSE); 
    glEnable(GL_DEPTH_CLAMP); 
    glStencilFunc(GL_ALWAYS, 0, 0xFF); 
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
    glDepthMask(GL_FALSE); 
    glEnable(GL_DEPTH_CLAMP); 
    glDepthFunc(GL_LEQUAL); 
    glDisable(GL_CULL_FACE);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP); 
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP); 

    glUseProgram(volumeShader->ID);
    glUniform3fv(glGetUniformLocation(volumeShader->ID, "lightPos"), 1, glm::value_ptr(lightPos));

    for (const auto& obj : scene->objects) {
        if (obj.name == "Piso" || obj.name == "Techo" || obj.name == "Pared Izquierda" || 
            obj.name == "Pared Derecha" || obj.name == "Pared del Frente" || obj.name == "Pared Trasera") {
            continue; 
        }

        glm::mat4 modelMatrix = obj.getModelMatrix();
        if (obj.type == MeshType::GLTF) {
            renderer->render(static_cast<const GLTFManager*>(obj.meshPointer), volumeShader, modelMatrix, view, projection);
        } else if (obj.type == MeshType::REVOLUTION_SOLID) {
            renderer->render(static_cast<const Mesh*>(obj.meshPointer), volumeShader, modelMatrix, view, projection);
        }
    }

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 
    glStencilMask(0x00); 
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glEnable(GL_BLEND);
    glUseProgram(flatShader->ID);
    glUniform4f(glGetUniformLocation(flatShader->ID, "objectColor"), 0.1f, 0.1f, 0.1f, 0.7f); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_CLAMP); 

    static unsigned int screenQuadVAO = 0;
    static unsigned int screenQuadVBO;
    if (screenQuadVAO == 0) {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f
        };
        glGenVertexArrays(1, &screenQuadVAO);
        glGenBuffers(1, &screenQuadVBO);
        glBindVertexArray(screenQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(flatShader->ID, "model"), 1, GL_FALSE, glm::value_ptr(identityMatrix));
    glUniformMatrix4fv(glGetUniformLocation(flatShader->ID, "view"), 1, GL_FALSE, glm::value_ptr(identityMatrix));
    glUniformMatrix4fv(glGetUniformLocation(flatShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(identityMatrix));
    
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_LESS);

    if (showShadowVolumes) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUseProgram(volumeShader->ID);
        for (const auto& obj : scene->objects) {
            if (obj.name == "Piso" || obj.name == "Techo" || obj.name == "Pared Izquierda" || 
                obj.name == "Pared Derecha" || obj.name == "Pared del Frente" || obj.name == "Pared Trasera") {
                continue; 
            }
            glm::mat4 modelMatrix = obj.getModelMatrix();
            if (obj.type == MeshType::GLTF) {
                renderer->render(static_cast<const GLTFManager*>(obj.meshPointer), volumeShader, modelMatrix, view, projection);
            } else if (obj.type == MeshType::REVOLUTION_SOLID) {
                renderer->render(static_cast<const Mesh*>(obj.meshPointer), volumeShader, modelMatrix, view, projection);
            }
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
    }
}
