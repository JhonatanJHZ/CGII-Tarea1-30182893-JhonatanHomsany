#include "../include/tools/GLTFManager.h"
#include <iostream>

GLTFManager::GLTFManager() {
    
}

GLTFManager::~GLTFManager() {
    for (auto& prim : primitives) {
        if (prim.VAO) glDeleteVertexArrays(1, &prim.VAO);
        if (prim.VBO) glDeleteBuffers(1, &prim.VBO);
        if (prim.EBO) glDeleteBuffers(1, &prim.EBO);
    }
}

bool GLTFManager::loadModel(const std::string& filename) {
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

    for (uint32_t m = 0; m < model->meshes_count; ++m) {
        const tg3_mesh& mesh = model->meshes[m];
        for (uint32_t p = 0; p < mesh.primitives_count; ++p) {
            const tg3_primitive& prim = mesh.primitives[p];
            
            int posAccessorIdx = -1;
            int normAccessorIdx = -1;
            for (uint32_t a = 0; a < prim.attributes_count; ++a) {
                const tg3_str_int_pair& attr = prim.attributes[a];
                std::string_view key(attr.key.data, attr.key.len);
                if (key == "POSITION") {
                    posAccessorIdx = attr.value;
                } else if (key == "NORMAL") {
                    normAccessorIdx = attr.value;
                }
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

            std::vector<Vertex> vertices;
            vertices.resize(posAccessor->count);

            for (uint64_t i = 0; i < posAccessor->count; ++i) {
                Vertex v;
                if (posData) {
                    const float* p = reinterpret_cast<const float*>(posData + i * posStride);
                    v.position = glm::vec3(p[0], p[1], p[2]);
                } else {
                    v.position = glm::vec3(0.0f);
                }

                if (normData) {
                    const float* n = reinterpret_cast<const float*>(normData + i * normStride);
                    v.normal = glm::vec3(n[0], n[1], n[2]);
                } else {
                    v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
                }
                vertices[i] = v;
            }

            GLTFPrimitive gltfPrim;
            gltfPrim.vertexCount = (unsigned int)posAccessor->count;
            gltfPrim.mode = (prim.mode == -1) ? GL_TRIANGLES : prim.mode;

            std::vector<unsigned int> indices;
            if (prim.indices != -1) {
                const tg3_accessor* idxAccessor = &model->accessors[prim.indices];
                const tg3_buffer_view* idxBV = idxAccessor->buffer_view >= 0 ? &model->buffer_views[idxAccessor->buffer_view] : nullptr;
                const tg3_buffer* idxBuffer = idxBV ? &model->buffers[idxBV->buffer] : nullptr;
                const uint8_t* idxData = idxBuffer ? idxBuffer->data.data + idxBV->byte_offset + idxAccessor->byte_offset : nullptr;
                
                int componentSize = 0;
                if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) componentSize = 1;
                else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) componentSize = 2;
                else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) componentSize = 4;

                int idxStride = idxBV ? (idxBV->byte_stride > 0 ? idxBV->byte_stride : componentSize) : componentSize;

                if (idxData) {
                    indices.resize(idxAccessor->count);
                    for (uint64_t i = 0; i < idxAccessor->count; ++i) {
                        unsigned int val = 0;
                        const uint8_t* p = idxData + i * idxStride;
                        if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            val = *reinterpret_cast<const uint8_t*>(p);
                        } else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            val = *reinterpret_cast<const uint16_t*>(p);
                        } else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                            val = *reinterpret_cast<const uint32_t*>(p);
                        }
                        indices[i] = val;
                    }
                    gltfPrim.hasIndices = true;
                    gltfPrim.indexCount = (unsigned int)idxAccessor->count;
                }
            }

            glGenVertexArrays(1, &gltfPrim.VAO);
            glGenBuffers(1, &gltfPrim.VBO);

            glBindVertexArray(gltfPrim.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, gltfPrim.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

            if (gltfPrim.hasIndices) {
                glGenBuffers(1, &gltfPrim.EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gltfPrim.EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            }

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            if (gltfPrim.hasIndices) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

            primitives.push_back(gltfPrim);
        }
    }
}

void GLTFManager::draw() const {
    for (const auto& prim : primitives) {
        if (prim.VAO == 0) continue;

        glBindVertexArray(prim.VAO);
        if (prim.hasIndices) {
            glDrawElements(prim.mode, prim.indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(prim.mode, 0, prim.vertexCount);
        }
    }
    glBindVertexArray(0);
}