
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/contrib/")
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/contrib/moggle/include/")

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(opengl ${OPENGL_LIBRARIES})
set(libs ${opengl} png)

add_subdirectory("src")
