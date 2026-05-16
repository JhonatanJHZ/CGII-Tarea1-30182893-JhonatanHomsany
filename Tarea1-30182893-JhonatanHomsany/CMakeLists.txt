# CMakeList.txt : CMake project for Tarea1-30182893-JhonatanHomsany, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Tarea1-30182893-JhonatanHomsany "Tarea1-30182893-JhonatanHomsany.cpp" "Tarea1-30182893-JhonatanHomsany.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Tarea1-30182893-JhonatanHomsany PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
