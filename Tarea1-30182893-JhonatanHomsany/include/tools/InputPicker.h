#pragma once
#include <glm/glm.hpp>
#include "../Ray.h"
#include "../SceneObject.h"
#include "../Scene.h"
class InputPicker {
public:
    Ray* ray;
    SceneObject* hit_object;
    float hit_t;
    InputPicker();
    ~InputPicker();
    void pick(double mouseX, double mouseY, int width, int height, 
              const glm::mat4& view, const glm::mat4& projection, 
              Scene* scene);
    SceneObject* getHitObject();
    Ray* getRay();
};
