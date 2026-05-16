include(FetchContent)

FetchContent_declare(
	glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 1.0.3
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)