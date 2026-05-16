#include "GLTFManager.h"
#include <iostream>

GLTFManager::GLTFManager() {
    
}

GLTFManager::~GLTFManager() {
    
}

bool GLTFManager::loadModel(const string& filename) {
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