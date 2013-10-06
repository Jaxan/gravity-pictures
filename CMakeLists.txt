
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/contrib/")
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/contrib/moggle/include/")
include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/contrib/simple-threading/include/")

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(opengl ${OPENGL_LIBRARIES})
set(libs ${opengl} png)

add_subdirectory("src")
