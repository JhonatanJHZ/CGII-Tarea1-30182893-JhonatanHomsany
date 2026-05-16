#pragma once

#include "tiny_gltf_v3.h"
#include <vector>
#include <string>

using namespace std;

class GLTFManager {
    public:
        GLTFManager();
        ~GLTFManager();
        
        bool loadModel(const string& filename);

    private:
        tinygltf3::Model model;
};