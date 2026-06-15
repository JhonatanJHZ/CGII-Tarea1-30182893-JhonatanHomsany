# Force CMake Cache Refresh 1
cmake_minimum_required (VERSION 3.10)
project ("Tarea1-30182893-JhonatanHomsany")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("vendor/glad")
add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
add_subdirectory("vendor/imgui")
add_subdirectory("vendor/tinygltf")

add_executable(Tarea1 
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Tarea1-30182893-JhonatanHomsany.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/GLTFManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/UIManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/GLFWManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Renderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Camera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Application.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Scene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/RevolutionSolidGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/BasicShapesGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Ray.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/InputPicker.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/ShadowManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/TextureManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/tools/tinyfiledialogs.c"
)

set_target_properties(Tarea1 PROPERTIES 
    OUTPUT_NAME "Tarea1-30182893-JhonatanHomsany"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(Tarea1 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui"
)

target_link_libraries(Tarea1 PRIVATE 
    glad_gl46
    glfw 
    glm 
    tinygltf
    imgui_lib
    Comdlg32
    Ole32
)