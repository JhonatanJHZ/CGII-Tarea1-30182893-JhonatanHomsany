cmake_minimum_required (VERSION 3.10)
project ("Tarea1-30182893-JhonatanHomsany")

add_subdirectory("vendor/glad")
add_subdirectory("vendor/glfw")
add_subdirectory("vendor/glm")
add_subdirectory("vendor/imgui")

add_executable(Tarea1 
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/src/Tarea1-30182893-JhonatanHomsany.cpp"
)

set_target_properties(Tarea1 PROPERTIES OUTPUT_NAME "Tarea1-30182893-JhonatanHomsany")

target_include_directories(Tarea1 PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/Tarea1-30182893-JhonatanHomsany/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui"
)

target_link_libraries(Tarea1 PRIVATE 
    glad_gl46
    glfw 
    glm 
    imgui_lib
)