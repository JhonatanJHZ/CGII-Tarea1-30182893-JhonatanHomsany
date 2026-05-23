add_library(tinygltf STATIC src/tiny_gltf_v3.c)
target_compile_definitions(tinygltf PUBLIC TINYGLTF3_ENABLE_FS)
target_include_directories(tinygltf PUBLIC include)
