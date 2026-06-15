#include "../include/tools/GLTFManager.h"
#include <iostream>
#include <stb_image.h>
#include "../include/Shader.h"

GLTFManager::GLTFManager() {
}

GLTFManager::~GLTFManager() {
    for (auto& prim : primitives) {
        if (prim.VAO) glDeleteVertexArrays(1, &prim.VAO);
        if (prim.VBO) glDeleteBuffers(1, &prim.VBO);
        if (prim.EBO) glDeleteBuffers(1, &prim.EBO);
    }
    for (unsigned int tex : glTextures) {
        if (tex) glDeleteTextures(1, &tex);
    }
}

bool GLTFManager::loadModel(const std::string& filename) {
    modelPath = filename;
    tinygltf3::ErrorStack errors;
    tg3_error_code err = tinygltf3::parse_file(model, errors, filename.c_str());
    if (err != TG3_OK) {
        for (uint32_t i = 0; i < errors.count(); i++) {
            const tg3_error_entry* entry = errors.entry(i);
            fprintf(stderr, "[%d] %s\n", (int)entry->severity,
                    entry->message ? entry->message : "(null)");
        }
        return false;
    }
    return true;
}

void GLTFManager::setupGL() {
    if (model->meshes_count == 0) return;
    
    cachedVertices.clear();

    glTextures.resize(model->textures_count, 0);
    for (uint32_t i = 0; i < model->textures_count; ++i) {
        const tg3_texture& tex = model->textures[i];
        if (tex.source >= 0 && (uint32_t)tex.source < model->images_count) {
            const tg3_image& img = model->images[tex.source];
            
            int width = 0, height = 0, channels = 0;
            unsigned char* pixels = nullptr;
            
            stbi_set_flip_vertically_on_load(false);
            
            if (img.buffer_view >= 0) {
                const tg3_buffer_view& bv = model->buffer_views[img.buffer_view];
                const tg3_buffer& buf = model->buffers[bv.buffer];
                const uint8_t* rawData = buf.data.data + bv.byte_offset;
                pixels = stbi_load_from_memory(rawData, (int)bv.byte_length, &width, &height, &channels, 0);
            } else if (img.image.data != nullptr && img.image.count > 0 && img.as_is) {
                pixels = stbi_load_from_memory(img.image.data, (int)img.image.count, &width, &height, &channels, 0);
            } else if (img.image.data != nullptr && img.width > 0 && img.height > 0 && !img.as_is) {
                pixels = const_cast<unsigned char*>(img.image.data);
                width = img.width;
                height = img.height;
                channels = img.component;
            } else if (img.uri.len > 0) {
                std::string uri(img.uri.data, img.uri.len);
                std::string basePath = modelPath.substr(0, modelPath.find_last_of("/\\") + 1);
                std::string texPath = basePath + uri;
                pixels = stbi_load(texPath.c_str(), &width, &height, &channels, 0);
            }

            if (!pixels) {
                std::cerr << "Failed to load image for texture " << i << std::endl;
                continue;
            }

            unsigned int textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            
            GLenum format = GL_RGBA;
            if (channels == 3) format = GL_RGB;
            else if (channels == 1) format = GL_RED;
            else if (channels == 2) format = GL_RG;
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glTextures[i] = textureID;
            
            if (pixels != img.image.data) {
                stbi_image_free(pixels);
            }
        }
    }

    materials.resize(model->materials_count);
    for (uint32_t i = 0; i < model->materials_count; ++i) {
        const tg3_material& mat = model->materials[i];
        PBRMaterial pbr;
        pbr.baseColorFactor = glm::vec4(mat.pbr_metallic_roughness.base_color_factor[0], 
                                        mat.pbr_metallic_roughness.base_color_factor[1],
                                        mat.pbr_metallic_roughness.base_color_factor[2],
                                        mat.pbr_metallic_roughness.base_color_factor[3]);
        pbr.metallicFactor = (float)mat.pbr_metallic_roughness.metallic_factor;
        pbr.roughnessFactor = (float)mat.pbr_metallic_roughness.roughness_factor;
        
        if (mat.pbr_metallic_roughness.base_color_texture.index >= 0 && (uint32_t)mat.pbr_metallic_roughness.base_color_texture.index < glTextures.size()) {
            pbr.baseColorTexture = glTextures[mat.pbr_metallic_roughness.base_color_texture.index];
        }
        if (mat.pbr_metallic_roughness.metallic_roughness_texture.index >= 0 && (uint32_t)mat.pbr_metallic_roughness.metallic_roughness_texture.index < glTextures.size()) {
            pbr.metallicRoughnessTexture = glTextures[mat.pbr_metallic_roughness.metallic_roughness_texture.index];
        }
        if (mat.normal_texture.index >= 0 && (uint32_t)mat.normal_texture.index < glTextures.size()) {
            pbr.normalTexture = glTextures[mat.normal_texture.index];
        }
        if (mat.occlusion_texture.index >= 0 && (uint32_t)mat.occlusion_texture.index < glTextures.size()) {
            pbr.occlusionTexture = glTextures[mat.occlusion_texture.index];
        }
        if (mat.emissive_texture.index >= 0 && (uint32_t)mat.emissive_texture.index < glTextures.size()) {
            pbr.emissiveTexture = glTextures[mat.emissive_texture.index];
        }
        pbr.emissiveFactor = glm::vec3(mat.emissive_factor[0], mat.emissive_factor[1], mat.emissive_factor[2]);
        materials[i] = pbr;
    }

    for (uint32_t m = 0; m < model->meshes_count; ++m) {
        const tg3_mesh& mesh = model->meshes[m];
        for (uint32_t p = 0; p < mesh.primitives_count; ++p) {
            const tg3_primitive& prim = mesh.primitives[p];
            int posAccessorIdx = -1;
            int normAccessorIdx = -1;
            int texAccessorIdx = -1;
            int colAccessorIdx = -1;
            
            for (uint32_t a = 0; a < prim.attributes_count; ++a) {
                const tg3_str_int_pair& attr = prim.attributes[a];
                std::string_view key(attr.key.data, attr.key.len);
                if (key == "POSITION") posAccessorIdx = attr.value;
                else if (key == "NORMAL") normAccessorIdx = attr.value;
                else if (key == "TEXCOORD_0") texAccessorIdx = attr.value;
                else if (key == "COLOR_0") colAccessorIdx = attr.value;
            }
            if (posAccessorIdx == -1) continue;

            const tg3_accessor* posAccessor = &model->accessors[posAccessorIdx];
            const tg3_buffer_view* posBV = posAccessor->buffer_view >= 0 ? &model->buffer_views[posAccessor->buffer_view] : nullptr;
            const tg3_buffer* posBuffer = posBV ? &model->buffers[posBV->buffer] : nullptr;
            const uint8_t* posData = posBuffer ? posBuffer->data.data + posBV->byte_offset + posAccessor->byte_offset : nullptr;
            int posStride = posBV ? (posBV->byte_stride > 0 ? posBV->byte_stride : 12) : 12;

            const tg3_accessor* normAccessor = normAccessorIdx >= 0 ? &model->accessors[normAccessorIdx] : nullptr;
            const tg3_buffer_view* normBV = (normAccessor && normAccessor->buffer_view >= 0) ? &model->buffer_views[normAccessor->buffer_view] : nullptr;
            const tg3_buffer* normBuffer = normBV ? &model->buffers[normBV->buffer] : nullptr;
            const uint8_t* normData = normBuffer ? normBuffer->data.data + normBV->byte_offset + normAccessor->byte_offset : nullptr;
            int normStride = normBV ? (normBV->byte_stride > 0 ? normBV->byte_stride : 12) : 12;

            const tg3_accessor* texAccessor = texAccessorIdx >= 0 ? &model->accessors[texAccessorIdx] : nullptr;
            const tg3_buffer_view* texBV = (texAccessor && texAccessor->buffer_view >= 0) ? &model->buffer_views[texAccessor->buffer_view] : nullptr;
            const tg3_buffer* texBuffer = texBV ? &model->buffers[texBV->buffer] : nullptr;
            const uint8_t* texData = texBuffer ? texBuffer->data.data + texBV->byte_offset + texAccessor->byte_offset : nullptr;
            int texStride = texBV ? (texBV->byte_stride > 0 ? texBV->byte_stride : (texAccessor->component_type == TG3_COMPONENT_TYPE_FLOAT ? 8 : (texAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT ? 4 : 2))) : 8;

            const tg3_accessor* colAccessor = colAccessorIdx >= 0 ? &model->accessors[colAccessorIdx] : nullptr;
            const tg3_buffer_view* colBV = (colAccessor && colAccessor->buffer_view >= 0) ? &model->buffer_views[colAccessor->buffer_view] : nullptr;
            const tg3_buffer* colBuffer = colBV ? &model->buffers[colBV->buffer] : nullptr;
            const uint8_t* colData = colBuffer ? colBuffer->data.data + colBV->byte_offset + colAccessor->byte_offset : nullptr;
            int colStride = colBV ? (colBV->byte_stride > 0 ? colBV->byte_stride : (colAccessor->component_type == TG3_COMPONENT_TYPE_FLOAT ? (colAccessor->type == TG3_TYPE_VEC3 ? 12 : 16) : (colAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT ? (colAccessor->type == TG3_TYPE_VEC3 ? 6 : 8) : (colAccessor->type == TG3_TYPE_VEC3 ? 3 : 4)))) : 16;

            std::vector<Vertex> temp_vertices;
            temp_vertices.resize(posAccessor->count);

            for (uint64_t i = 0; i < posAccessor->count; ++i) {
                Vertex v;
                if (posData) {
                    const float* p = reinterpret_cast<const float*>(posData + i * posStride);
                    v.position = glm::vec3(p[0], p[1], p[2]);
                } else v.position = glm::vec3(0.0f);
                if (normData) {
                    const float* n = reinterpret_cast<const float*>(normData + i * normStride);
                    v.normal = glm::vec3(n[0], n[1], n[2]);
                } else v.normal = glm::vec3(0.0f, 0.0f, 1.0f);

                if (texData) {
                    if (texAccessor->component_type == TG3_COMPONENT_TYPE_FLOAT) {
                        const float* t = reinterpret_cast<const float*>(texData + i * texStride);
                        v.textureCoordinates = glm::vec2(t[0], t[1]);
                    } else if (texAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        const uint8_t* t = texData + i * texStride;
                        v.textureCoordinates = glm::vec2(t[0]/255.f, t[1]/255.f);
                    } else if (texAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const uint16_t* t = reinterpret_cast<const uint16_t*>(texData + i * texStride);
                        v.textureCoordinates = glm::vec2(t[0]/65535.f, t[1]/65535.f);
                    } else {
                        v.textureCoordinates = glm::vec2(0.0f);
                    }
                } else v.textureCoordinates = glm::vec2(0.0f);
                if (colData) {
                    if (colAccessor->component_type == TG3_COMPONENT_TYPE_FLOAT) {
                        const float* c = reinterpret_cast<const float*>(colData + i * colStride);
                        v.color = (colAccessor->type == TG3_TYPE_VEC3) ? glm::vec4(c[0], c[1], c[2], 1.0f) : glm::vec4(c[0], c[1], c[2], c[3]);
                    } else if (colAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        const uint8_t* c = colData + i * colStride;
                        v.color = (colAccessor->type == TG3_TYPE_VEC3) ? glm::vec4(c[0]/255.f, c[1]/255.f, c[2]/255.f, 1.0f) : glm::vec4(c[0]/255.f, c[1]/255.f, c[2]/255.f, c[3]/255.f);
                    } else if (colAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const uint16_t* c = reinterpret_cast<const uint16_t*>(colData + i * colStride);
                        v.color = (colAccessor->type == TG3_TYPE_VEC3) ? glm::vec4(c[0]/65535.f, c[1]/65535.f, c[2]/65535.f, 1.0f) : glm::vec4(c[0]/65535.f, c[1]/65535.f, c[2]/65535.f, c[3]/65535.f);
                    } else v.color = glm::vec4(1.0f);
                } else v.color = glm::vec4(1.0f);

                if (prim.material >= 0 && prim.material < (int)materials.size()) {
                    v.color *= materials[prim.material].baseColorFactor;
                }

                temp_vertices[i] = v;
            }

            GLTFPrimitive gltfPrim;
            gltfPrim.vertexCount = (unsigned int)posAccessor->count;
            gltfPrim.mode = (prim.mode == -1) ? GL_TRIANGLES : prim.mode;
            gltfPrim.materialIndex = prim.material;

            std::vector<unsigned int> indices;
            if (prim.indices != -1) {
                const tg3_accessor* idxAccessor = &model->accessors[prim.indices];
                const tg3_buffer_view* idxBV = idxAccessor->buffer_view >= 0 ? &model->buffer_views[idxAccessor->buffer_view] : nullptr;
                const tg3_buffer* idxBuffer = idxBV ? &model->buffers[idxBV->buffer] : nullptr;
                const uint8_t* idxData = idxBuffer ? idxBuffer->data.data + idxBV->byte_offset + idxAccessor->byte_offset : nullptr;
                int componentSize = 1;
                if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) componentSize = 2;
                else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) componentSize = 4;
                int idxStride = idxBV ? (idxBV->byte_stride > 0 ? idxBV->byte_stride : componentSize) : componentSize;

                if (idxData) {
                    indices.resize(idxAccessor->count);
                    for (uint64_t i = 0; i < idxAccessor->count; ++i) {
                        unsigned int val = 0;
                        const uint8_t* p = idxData + i * idxStride;
                        if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) val = *reinterpret_cast<const uint8_t*>(p);
                        else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) val = *reinterpret_cast<const uint16_t*>(p);
                        else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) val = *reinterpret_cast<const uint32_t*>(p);
                        indices[i] = val;
                        
                        if (val < temp_vertices.size()) {
                            cachedVertices.push_back(temp_vertices[val]);
                        }
                    }
                    gltfPrim.hasIndices = true;
                    gltfPrim.indexCount = (unsigned int)idxAccessor->count;
                }
            } else {
                cachedVertices.insert(cachedVertices.end(), temp_vertices.begin(), temp_vertices.end());
            }

            glGenVertexArrays(1, &gltfPrim.VAO);
            glGenBuffers(1, &gltfPrim.VBO);
            glBindVertexArray(gltfPrim.VAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, gltfPrim.VBO);
            glBufferData(GL_ARRAY_BUFFER, temp_vertices.size() * sizeof(Vertex), temp_vertices.data(), GL_STATIC_DRAW);
            
            if (gltfPrim.hasIndices) {
                glGenBuffers(1, &gltfPrim.EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gltfPrim.EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            }
            
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoordinates));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
            
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            if (gltfPrim.hasIndices) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            primitives.push_back(gltfPrim);
        }
    }
}

void GLTFManager::draw(const class Shader* shader) const {
    for (const auto& prim : primitives) {
        if (prim.VAO == 0) continue;
        
        if (shader && prim.materialIndex >= 0 && prim.materialIndex < (int)materials.size()) {
            const PBRMaterial& mat = materials[prim.materialIndex];
            
            glUniform3fv(glGetUniformLocation(shader->ID, "objectColor"), 1, glm::value_ptr(glm::vec3(mat.baseColorFactor)));
            glUniform1i(glGetUniformLocation(shader->ID, "useVertexColor"), 1);
            glUniform1f(glGetUniformLocation(shader->ID, "metallicValue"), mat.metallicFactor);
            glUniform1f(glGetUniformLocation(shader->ID, "roughnessValue"), mat.roughnessFactor);
            glUniform1f(glGetUniformLocation(shader->ID, "aoValue"), 1.0f);

            bool hasAlbedo = mat.baseColorTexture != 0;
            glUniform1i(glGetUniformLocation(shader->ID, "hasAlbedoMap"), hasAlbedo);
            if (hasAlbedo) {
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, mat.baseColorTexture);
                glUniform1i(glGetUniformLocation(shader->ID, "albedoMap"), 4);
            }

            bool hasNormal = mat.normalTexture != 0;
            glUniform1i(glGetUniformLocation(shader->ID, "hasNormalMap"), hasNormal);
            if (hasNormal) {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, mat.normalTexture);
                glUniform1i(glGetUniformLocation(shader->ID, "normalMap"), 2);
            }

            bool hasMetallicRoughness = mat.metallicRoughnessTexture != 0;
            glUniform1i(glGetUniformLocation(shader->ID, "hasMetallicMap"), hasMetallicRoughness);
            glUniform1i(glGetUniformLocation(shader->ID, "hasRoughnessMap"), hasMetallicRoughness);
            if (hasMetallicRoughness) {
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, mat.metallicRoughnessTexture);
                glUniform1i(glGetUniformLocation(shader->ID, "metallicMap"), 5);
                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_2D, mat.metallicRoughnessTexture);
                glUniform1i(glGetUniformLocation(shader->ID, "roughnessMap"), 6);
            }

            bool hasAo = mat.occlusionTexture != 0;
            glUniform1i(glGetUniformLocation(shader->ID, "hasAoMap"), hasAo);
            if (hasAo) {
                glActiveTexture(GL_TEXTURE7);
                glBindTexture(GL_TEXTURE_2D, mat.occlusionTexture);
                glUniform1i(glGetUniformLocation(shader->ID, "aoMap"), 7);
            }
        }

        glBindVertexArray(prim.VAO);
        if (prim.hasIndices) {
            glDrawElements(prim.mode, prim.indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(prim.mode, 0, prim.vertexCount);
        }
    }
    glBindVertexArray(0);
}

std::vector<Vertex> GLTFManager::getVertices() const {
    return cachedVertices;
}
