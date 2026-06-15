#define GLM_ENABLE_EXPERIMENTAL
#include "../include/Scene.h"
#include "../include/Renderer.h"
#include "../include/tools/GLTFManager.h"
#include "../include/Mesh.h"
#include "../include/Shader.h"
#include "../include/Lighting.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tiny_gltf_v3.h>
#include <iostream>
#include <string_view>
Scene::Scene() {}
Scene::~Scene() {
    for (auto& obj : objects) {
        if (obj.meshPointer) {
            if (obj.type == MeshType::GLTF) {
                delete static_cast<GLTFManager*>(obj.meshPointer);
            } else if (obj.type == MeshType::REVOLUTION_SOLID) {
                delete static_cast<Mesh*>(obj.meshPointer);
            }
            obj.meshPointer = nullptr;
        }
    }
}
void Scene::addObject(const SceneObject& obj) {
    objects.push_back(obj);
}
void Scene::removeObject(size_t index) {
    if (index < objects.size()) {
        auto& obj = objects[index];
        if (obj.meshPointer) {
            if (obj.type == MeshType::GLTF) {
                delete static_cast<GLTFManager*>(obj.meshPointer);
            } else if (obj.type == MeshType::REVOLUTION_SOLID) {
                delete static_cast<Mesh*>(obj.meshPointer);
            }
            obj.meshPointer = nullptr;
        }
        objects.erase(objects.begin() + index);
    }
}
void Scene::draw(const Shader* shader, const Renderer* renderer, 
                 const glm::mat4& view, const glm::mat4& projection,
                 const Lighting* lighting, const glm::vec3& viewPos) const {
    glUseProgram(shader->ID);
    glUniform3fv(glGetUniformLocation(shader->ID, "viewPos"), 1, glm::value_ptr(viewPos));
    if (lighting && !lighting->lights.empty()) {
        const Light& mainLight = lighting->lights[0];
        glUniform3fv(glGetUniformLocation(shader->ID, "lightPos"), 1, glm::value_ptr(mainLight.position));
        glUniform3fv(glGetUniformLocation(shader->ID, "lightColor"), 1, glm::value_ptr(mainLight.color));
        glUniform1f(glGetUniformLocation(shader->ID, "lightIntensity"), mainLight.intensity);
        glUniform1f(glGetUniformLocation(shader->ID, "ambientIntensity"), mainLight.ambientIntensity);
        glUniform1f(glGetUniformLocation(shader->ID, "specularStrength"), mainLight.specularStrength);
        glUniform1f(glGetUniformLocation(shader->ID, "shininess"), mainLight.shininess);
        glUniform1i(glGetUniformLocation(shader->ID, "shadingMode"), static_cast<int>(lighting->activeMode));
        glUniform1f(glGetUniformLocation(shader->ID, "exposure"), lighting->exposure);
    } else {
        glm::vec3 defaultPos(2.0f, 4.0f, 5.0f);
        glm::vec3 defaultColor(1.0f);
        glUniform3fv(glGetUniformLocation(shader->ID, "lightPos"), 1, glm::value_ptr(defaultPos));
        glUniform3fv(glGetUniformLocation(shader->ID, "lightColor"), 1, glm::value_ptr(defaultColor));
        glUniform1f(glGetUniformLocation(shader->ID, "lightIntensity"), 1.0f);
        glUniform1f(glGetUniformLocation(shader->ID, "ambientIntensity"), 0.15f);
        glUniform1f(glGetUniformLocation(shader->ID, "specularStrength"), 0.5f);
        glUniform1f(glGetUniformLocation(shader->ID, "shininess"), 32.0f);
        glUniform1i(glGetUniformLocation(shader->ID, "shadingMode"), 3);
        glUniform1f(glGetUniformLocation(shader->ID, "exposure"), 2.0f);
    }
    for (const auto& obj : objects) {
        if (!obj.meshPointer) continue;
        glm::mat4 modelMatrix = obj.getModelMatrix();
        glUniform3fv(glGetUniformLocation(shader->ID, "objectColor"), 1, glm::value_ptr(obj.color));
        
        glUniform1i(glGetUniformLocation(shader->ID, "textureType"), static_cast<int>(obj.textureType));
        
        bool hasNormal = (obj.normalMapID != 0);
        bool hasBump = (obj.bumpMapID != 0);
        bool hasAlbedo = (obj.albedoMapID != 0);
        glUniform1i(glGetUniformLocation(shader->ID, "hasNormalMap"), hasNormal);
        glUniform1i(glGetUniformLocation(shader->ID, "hasBumpMap"), hasBump);
        glUniform1i(glGetUniformLocation(shader->ID, "hasAlbedoMap"), hasAlbedo);

        if (hasNormal) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, obj.normalMapID);
            glUniform1i(glGetUniformLocation(shader->ID, "normalMap"), 2);
        }
        if (hasBump) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, obj.bumpMapID);
            glUniform1i(glGetUniformLocation(shader->ID, "bumpMap"), 3);
        }
        if (hasAlbedo) {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, obj.albedoMapID);
            glUniform1i(glGetUniformLocation(shader->ID, "albedoMap"), 4);
        }
        
        bool hasMetallic = (obj.metallicMapID != 0);
        bool hasRoughness = (obj.roughnessMapID != 0);
        bool hasAo = (obj.aoMapID != 0);
        
        glUniform1i(glGetUniformLocation(shader->ID, "hasMetallicMap"), hasMetallic);
        glUniform1i(glGetUniformLocation(shader->ID, "hasRoughnessMap"), hasRoughness);
        glUniform1i(glGetUniformLocation(shader->ID, "hasAoMap"), hasAo);
        
        glUniform1f(glGetUniformLocation(shader->ID, "metallicValue"), obj.metallicValue);
        glUniform1f(glGetUniformLocation(shader->ID, "roughnessValue"), obj.roughnessValue);
        glUniform1f(glGetUniformLocation(shader->ID, "aoValue"), obj.aoValue);

        if (hasMetallic) {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, obj.metallicMapID);
            glUniform1i(glGetUniformLocation(shader->ID, "metallicMap"), 5);
        }
        if (hasRoughness) {
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, obj.roughnessMapID);
            glUniform1i(glGetUniformLocation(shader->ID, "roughnessMap"), 6);
        }
        if (hasAo) {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, obj.aoMapID);
            glUniform1i(glGetUniformLocation(shader->ID, "aoMap"), 7);
        }

        if (obj.type == MeshType::GLTF) {
            renderer->render(static_cast<const GLTFManager*>(obj.meshPointer), shader, modelMatrix, view, projection);
        } else if (obj.type == MeshType::REVOLUTION_SOLID) {
            glUniform1i(glGetUniformLocation(shader->ID, "useVertexColor"), 0);
            renderer->render(static_cast<const Mesh*>(obj.meshPointer), shader, modelMatrix, view, projection);
        }
    }
}
bool Scene::saveScene(const std::string& filepath) const {
    std::vector<tg3_node> nodes;
    std::vector<tg3_mesh> meshes;
    std::vector<tg3_material> materials;
    std::vector<tg3_accessor> accessors;
    std::vector<tg3_buffer_view> buffer_views;
    std::vector<tg3_buffer> buffers;
    std::vector<uint8_t> binBuffer;
    std::vector<std::vector<tg3_str_int_pair>> primitive_attributes;
    std::vector<std::vector<tg3_primitive>> mesh_primitives;
    std::vector<std::string> string_pool;
    auto make_tg3_str = [&](const std::string& s) {
        string_pool.push_back(s);
        tg3_str t;
        t.data = string_pool.back().c_str();
        t.len = static_cast<uint32_t>(string_pool.back().size());
        return t;
    };
    for (const auto& obj : objects) {
        if (!obj.meshPointer) continue;
        tg3_material mat;
        memset(&mat, 0, sizeof(mat));
        mat.name = make_tg3_str(obj.name + "_material");
        mat.pbr_metallic_roughness.base_color_factor[0] = obj.color.r;
        mat.pbr_metallic_roughness.base_color_factor[1] = obj.color.g;
        mat.pbr_metallic_roughness.base_color_factor[2] = obj.color.b;
        mat.pbr_metallic_roughness.base_color_factor[3] = 1.0;
        mat.pbr_metallic_roughness.metallic_factor = 0.0;
        mat.pbr_metallic_roughness.roughness_factor = 0.8;
        materials.push_back(mat);
        int mat_index = static_cast<int>(materials.size() - 1);
        std::vector<std::vector<Vertex>> primitives_vertices;
        if (obj.type == MeshType::REVOLUTION_SOLID) {
            Mesh* m = static_cast<Mesh*>(obj.meshPointer);
            primitives_vertices.push_back(m->getVertices());
        } else if (obj.type == MeshType::GLTF) {
            GLTFManager* gltf = static_cast<GLTFManager*>(obj.meshPointer);
            const tinygltf3::Model& model = gltf->model;
            if (model->meshes_count > 0) {
                for (uint32_t m_idx = 0; m_idx < model->meshes_count; ++m_idx) {
                    const tg3_mesh& mesh = model->meshes[m_idx];
                    for (uint32_t p_idx = 0; p_idx < mesh.primitives_count; ++p_idx) {
                        const tg3_primitive& prim = mesh.primitives[p_idx];
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
                        std::vector<Vertex> temp_vertices;
                        temp_vertices.resize(posAccessor->count);
                        for (uint64_t v_idx = 0; v_idx < posAccessor->count; ++v_idx) {
                            Vertex v;
                            if (posData) {
                                const float* p = reinterpret_cast<const float*>(posData + v_idx * posStride);
                                v.position = glm::vec3(p[0], p[1], p[2]);
                            } else {
                                v.position = glm::vec3(0.0f);
                            }
                            if (normData) {
                                const float* n = reinterpret_cast<const float*>(normData + v_idx * normStride);
                                v.normal = glm::vec3(n[0], n[1], n[2]);
                            } else {
                                v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
                            }
                            temp_vertices[v_idx] = v;
                        }
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
                            std::vector<Vertex> unindexed_vertices;
                            if (idxData) {
                                unindexed_vertices.reserve(idxAccessor->count);
                                for (uint64_t idx_i = 0; idx_i < idxAccessor->count; ++idx_i) {
                                    unsigned int idx_val = 0;
                                    const uint8_t* p = idxData + idx_i * idxStride;
                                    if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) {
                                        idx_val = *reinterpret_cast<const uint8_t*>(p);
                                    } else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                                        idx_val = *reinterpret_cast<const uint16_t*>(p);
                                    } else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                                        idx_val = *reinterpret_cast<const uint32_t*>(p);
                                    }
                                    if (idx_val < temp_vertices.size()) {
                                        unindexed_vertices.push_back(temp_vertices[idx_val]);
                                    }
                                }
                            }
                            primitives_vertices.push_back(unindexed_vertices);
                        } else {
                            primitives_vertices.push_back(temp_vertices);
                        }
                    }
                }
            }
        }
        std::vector<tg3_primitive> prims_list;
        for (const auto& vertices : primitives_vertices) {
            if (vertices.empty()) continue;
            uint64_t offset = binBuffer.size();
            uint64_t size_bytes = vertices.size() * sizeof(Vertex);
            binBuffer.resize(offset + size_bytes);
            memcpy(binBuffer.data() + offset, vertices.data(), size_bytes);
            tg3_buffer_view bv;
            memset(&bv, 0, sizeof(bv));
            bv.buffer = 0;
            bv.byte_offset = offset;
            bv.byte_length = size_bytes;
            bv.byte_stride = sizeof(Vertex);
            bv.target = 34962;
            buffer_views.push_back(bv);
            int bv_index = static_cast<int>(buffer_views.size() - 1);
            glm::vec3 min_p = vertices[0].position;
            glm::vec3 max_p = vertices[0].position;
            for (const auto& v : vertices) {
                min_p = glm::min(min_p, v.position);
                max_p = glm::max(max_p, v.position);
            }
            tg3_accessor pos_acc;
            memset(&pos_acc, 0, sizeof(pos_acc));
            pos_acc.buffer_view = bv_index;
            pos_acc.byte_offset = offsetof(Vertex, position);
            pos_acc.component_type = TG3_COMPONENT_TYPE_FLOAT;
            pos_acc.count = vertices.size();
            pos_acc.type = TG3_TYPE_VEC3;
            double* min_vals = new double[3]{ min_p.x, min_p.y, min_p.z };
            double* max_vals = new double[3]{ max_p.x, max_p.y, max_p.z };
            pos_acc.min_values = min_vals;
            pos_acc.min_values_count = 3;
            pos_acc.max_values = max_vals;
            pos_acc.max_values_count = 3;
            accessors.push_back(pos_acc);
            int pos_acc_idx = static_cast<int>(accessors.size() - 1);
            tg3_accessor norm_acc;
            memset(&norm_acc, 0, sizeof(norm_acc));
            norm_acc.buffer_view = bv_index;
            norm_acc.byte_offset = offsetof(Vertex, normal);
            norm_acc.component_type = TG3_COMPONENT_TYPE_FLOAT;
            norm_acc.count = vertices.size();
            norm_acc.type = TG3_TYPE_VEC3;
            accessors.push_back(norm_acc);
            int norm_acc_idx = static_cast<int>(accessors.size() - 1);
            std::vector<tg3_str_int_pair> attrs;
            tg3_str_int_pair attr_pos;
            attr_pos.key = make_tg3_str("POSITION");
            attr_pos.value = pos_acc_idx;
            attrs.push_back(attr_pos);
            tg3_str_int_pair attr_norm;
            attr_norm.key = make_tg3_str("NORMAL");
            attr_norm.value = norm_acc_idx;
            attrs.push_back(attr_norm);
            primitive_attributes.push_back(attrs);
            tg3_primitive prim;
            memset(&prim, 0, sizeof(prim));
            prim.attributes = primitive_attributes.back().data();
            prim.attributes_count = static_cast<uint32_t>(primitive_attributes.back().size());
            prim.material = mat_index;
            prim.indices = -1;
            prim.mode = TG3_MODE_TRIANGLES;
            prims_list.push_back(prim);
        }
        if (prims_list.empty()) continue;
        mesh_primitives.push_back(prims_list);
        tg3_mesh mesh;
        memset(&mesh, 0, sizeof(mesh));
        mesh.name = make_tg3_str(obj.name + "_mesh");
        mesh.primitives = mesh_primitives.back().data();
        mesh.primitives_count = static_cast<uint32_t>(mesh_primitives.back().size());
        meshes.push_back(mesh);
        int mesh_index = static_cast<int>(meshes.size() - 1);
        tg3_node node;
        memset(&node, 0, sizeof(node));
        std::string prefix = (obj.type == MeshType::GLTF) ? "[GLTF] " : "[REV] ";
        node.name = make_tg3_str(prefix + obj.name);
        node.mesh = mesh_index;
        node.camera = -1;
        node.skin = -1;
        node.light = -1;
        node.translation[0] = obj.position.x;
        node.translation[1] = obj.position.y;
        node.translation[2] = obj.position.z;
        node.scale[0] = obj.scale.x;
        node.scale[1] = obj.scale.y;
        node.scale[2] = obj.scale.z;
        glm::quat qX = glm::angleAxis(glm::radians(obj.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat qY = glm::angleAxis(glm::radians(obj.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat qZ = glm::angleAxis(glm::radians(obj.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat q = qZ * qY * qX;
        node.rotation[0] = q.x;
        node.rotation[1] = q.y;
        node.rotation[2] = q.z;
        node.rotation[3] = q.w;
        nodes.push_back(node);
    }
    tg3_model rawModel;
    memset(&rawModel, 0, sizeof(rawModel));
    rawModel.asset.version = make_tg3_str("2.0");
    rawModel.asset.generator = make_tg3_str("Antigravity Scene Exporter");
    std::vector<int32_t> scene_nodes;
    for (int node_idx = 0; node_idx < static_cast<int>(nodes.size()); ++node_idx) {
        scene_nodes.push_back(node_idx);
    }
    tg3_scene scene_struct;
    memset(&scene_struct, 0, sizeof(scene_struct));
    scene_struct.name = make_tg3_str("MainScene");
    scene_struct.nodes = scene_nodes.data();
    scene_struct.nodes_count = static_cast<uint32_t>(scene_nodes.size());
    std::vector<tg3_scene> scenes_list;
    scenes_list.push_back(scene_struct);
    rawModel.scenes = scenes_list.data();
    rawModel.scenes_count = static_cast<uint32_t>(scenes_list.size());
    rawModel.default_scene = 0;
    rawModel.nodes = nodes.data();
    rawModel.nodes_count = static_cast<uint32_t>(nodes.size());
    rawModel.meshes = meshes.data();
    rawModel.meshes_count = static_cast<uint32_t>(meshes.size());
    rawModel.materials = materials.data();
    rawModel.materials_count = static_cast<uint32_t>(materials.size());
    rawModel.accessors = accessors.data();
    rawModel.accessors_count = static_cast<uint32_t>(accessors.size());
    rawModel.buffer_views = buffer_views.data();
    rawModel.buffer_views_count = static_cast<uint32_t>(buffer_views.size());
    tg3_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.name = make_tg3_str("BinaryBuffer");
    buf.data.data = binBuffer.data();
    buf.data.count = binBuffer.size();
    buffers.push_back(buf);
    rawModel.buffers = buffers.data();
    rawModel.buffers_count = static_cast<uint32_t>(buffers.size());
    tg3_write_options options;
    tg3_write_options_init(&options);
    options.write_binary = 1;
    options.embed_buffers = 0;
    tinygltf3::ErrorStack errors;
    tg3_error_code err = tg3_write_to_file(&rawModel, errors.get(), filepath.c_str(), static_cast<uint32_t>(filepath.size()), &options);
    for (auto& acc : accessors) {
        if (acc.min_values) delete[] acc.min_values;
        if (acc.max_values) delete[] acc.max_values;
    }
    if (err != TG3_OK) {
        std::cerr << "Error writing scene GLB: " << err << std::endl;
        for (uint32_t i = 0; i < errors.count(); i++) {
            const tg3_error_entry* entry = errors.entry(i);
            fprintf(stderr, "[%d] %s\n", (int)entry->severity, entry->message ? entry->message : "(null)");
        }
        return false;
    }
    return true;
}
bool Scene::loadScene(const std::string& filepath) {
    tinygltf3::Model model;
    tinygltf3::ErrorStack errors;
    tg3_error_code err = tinygltf3::parse_file(model, errors, filepath.c_str());
    if (err != TG3_OK) {
        std::cerr << "Error loading scene GLB: " << err << std::endl;
        for (uint32_t i = 0; i < errors.count(); i++) {
            const tg3_error_entry* entry = errors.entry(i);
            fprintf(stderr, "[%d] %s\n", (int)entry->severity, entry->message ? entry->message : "(null)");
        }
        return false;
    }
    for (auto& obj : objects) {
        if (obj.meshPointer) {
            if (obj.type == MeshType::GLTF) {
                delete static_cast<GLTFManager*>(obj.meshPointer);
            } else if (obj.type == MeshType::REVOLUTION_SOLID) {
                delete static_cast<Mesh*>(obj.meshPointer);
            }
            obj.meshPointer = nullptr;
        }
    }
    objects.clear();
    if (model->nodes_count == 0) return true;
    for (uint32_t n_idx = 0; n_idx < model->nodes_count; ++n_idx) {
        const tg3_node& node = model->nodes[n_idx];
        if (node.mesh == -1) continue;
        const tg3_mesh& mesh = model->meshes[node.mesh];
        std::vector<Vertex> vertices;
        glm::vec3 mat_color(0.8f);
        for (uint32_t p_idx = 0; p_idx < mesh.primitives_count; ++p_idx) {
            const tg3_primitive& prim = mesh.primitives[p_idx];
            if (prim.material >= 0 && prim.material < static_cast<int>(model->materials_count)) {
                const tg3_material& mat = model->materials[prim.material];
                mat_color.r = static_cast<float>(mat.pbr_metallic_roughness.base_color_factor[0]);
                mat_color.g = static_cast<float>(mat.pbr_metallic_roughness.base_color_factor[1]);
                mat_color.b = static_cast<float>(mat.pbr_metallic_roughness.base_color_factor[2]);
            }
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
            std::vector<Vertex> prim_vertices;
            prim_vertices.resize(posAccessor->count);
            for (uint64_t v_idx = 0; v_idx < posAccessor->count; ++v_idx) {
                Vertex v;
                if (posData) {
                    const float* p = reinterpret_cast<const float*>(posData + v_idx * posStride);
                    v.position = glm::vec3(p[0], p[1], p[2]);
                } else {
                    v.position = glm::vec3(0.0f);
                }
                if (normData) {
                    const float* n = reinterpret_cast<const float*>(normData + v_idx * normStride);
                    v.normal = glm::vec3(n[0], n[1], n[2]);
                } else {
                    v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
                }
                prim_vertices[v_idx] = v;
            }
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
                    for (uint64_t idx_i = 0; idx_i < idxAccessor->count; ++idx_i) {
                        unsigned int val = 0;
                        const uint8_t* p = idxData + idx_i * idxStride;
                        if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            val = *reinterpret_cast<const uint8_t*>(p);
                        } else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            val = *reinterpret_cast<const uint16_t*>(p);
                        } else if (idxAccessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT) {
                            val = *reinterpret_cast<const uint32_t*>(p);
                        }
                        if (val < prim_vertices.size()) {
                            vertices.push_back(prim_vertices[val]);
                        }
                    }
                }
            } else {
                vertices.insert(vertices.end(), prim_vertices.begin(), prim_vertices.end());
            }
        }
        if (vertices.empty()) continue;
        std::string full_name(node.name.data, node.name.len);
        MeshType type = MeshType::REVOLUTION_SOLID;
        std::string obj_name = full_name;
        if (full_name.rfind("[GLTF] ", 0) == 0) {
            type = MeshType::GLTF;
            obj_name = full_name.substr(7);
        } else if (full_name.rfind("[REV] ", 0) == 0) {
            type = MeshType::REVOLUTION_SOLID;
            obj_name = full_name.substr(6);
        }
        SceneObject newObj;
        newObj.name = obj_name;
        newObj.type = MeshType::REVOLUTION_SOLID;
        newObj.color = mat_color;
        newObj.position = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
        newObj.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
        glm::quat q(
            static_cast<float>(node.rotation[3]), 
            static_cast<float>(node.rotation[0]), 
            static_cast<float>(node.rotation[1]), 
            static_cast<float>(node.rotation[2])  
        );
        newObj.rotation = glm::degrees(glm::eulerAngles(q));
        Mesh* newMesh = new Mesh(vertices);
        newObj.meshPointer = newMesh;
        addObject(newObj);
    }
    return true;
}
