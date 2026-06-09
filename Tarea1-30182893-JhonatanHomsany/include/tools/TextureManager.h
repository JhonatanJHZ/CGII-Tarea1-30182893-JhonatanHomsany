#pragma once
#include <string>

enum class TextureType{
    NONE,
    CUBICAL,
    SPHERICAL,
    CYLINDRICAL,   
};

class TextureManager{
    public:

    TextureManager();
    ~TextureManager();

    static unsigned int loadTexture(const std::string& path);
};