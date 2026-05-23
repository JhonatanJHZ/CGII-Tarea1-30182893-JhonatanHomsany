#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstddef>
#include <vector>
#include <string>
#include "RevolutionSolidGenerator.h"


struct GLFWwindow;
class Scene;
class Lighting;
class Ray;
class InputPicker;

class UIManager {
    private:
        int selectedObjectIndex = -1;
        std::vector<ProfileSegment> currentSegments;
        int radialSegments = 32;
        int samplePointsPerSegment = 20;

        char importPathBuffer[256] = "../../../Tarea1-30182893-JhonatanHomsany/assets/Link_ Zelda Tear of the Kingdom.glb";
        char savePathBuffer[256] = "../../../Tarea1-30182893-JhonatanHomsany/assets/escena_guardada.glb";
        char loadPathBuffer[256] = "../../../Tarea1-30182893-JhonatanHomsany/assets/escena_guardada.glb";

        void loadCylinderPreset();
        void loadConePreset();
        void loadSpherePreset();

    public:
        UIManager(GLFWwindow* window);
        ~UIManager();
        void newFrame();
        void drawInspector(Scene* scene, Lighting* lighting, Ray* ray, InputPicker* picker);
        void render();
};