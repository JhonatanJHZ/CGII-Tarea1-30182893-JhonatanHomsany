add_library(glad_gl46 STATIC src/glad.c)

target_include_directories(glad_gl46 PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)