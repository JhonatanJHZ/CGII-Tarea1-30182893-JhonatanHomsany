#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstddef>
#include <vector>
#include <string>
#include "../tools/RevolutionSolidGenerator.h"
#include "../tools/ShadowManager.h"

struct GLFWwindow;
class Scene;
class Lighting;
class Ray;
class InputPicker;
class Camera;
enum class ShapeType;

class UIManager {
    private:
        int selectedObjectIndex = -1;
        std::vector<ProfileSegment> currentSegments;
        int radialSegments = 32;
        int samplePointsPerSegment = 20;

        char importPathBuffer[256] = "../../../Tarea1-30182893-JhonatanHomsany/assets/Link_ Zelda Tear of the Kingdom.glb";
        char savePathBuffer[256] = "../../../Tarea1-30182893-JhonatanHomsany/assets/escena_guardada.glb";
        char loadPathBuffer[256] = "../../../Tarea1-30182893-JhonatanHomsany/assets/escena_guardada.glb";

        void addInstructionsUI();
        void generateRevolutionSolid(Scene* scene, InputPicker* picker, ShapeType& activeShapeType);
        void addObjectGenerationUI(Scene* scene, InputPicker* picker, ShapeType& activeShapeType);
        void addPickerUI(Scene* scene, InputPicker* picker);
        void addIlluminationUI(Lighting* lighting);
        void addRaycastUI(Ray* ray);
        void addFileManagementUI(Scene* scene);
        void addCameraUI(std::vector<Camera*>& cameras, int& activeCameraIndex);
        void addShadowModesUI();

    public:
        UIManager(GLFWwindow* window);
        ~UIManager();
        void newFrame();
        void drawInspector(Scene* scene, Lighting* lighting, Ray* ray, InputPicker* picker, std::vector<Camera*>& cameras, int& activeCameraIndex);
        void render();
};