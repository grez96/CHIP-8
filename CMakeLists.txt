# CMake version
cmake_minimum_required(VERSION 3.0)
# project name
project(CHIP-8)

# dependency management
# git - terminate if not found
find_package(Git REQUIRED)

# OpenGL - terminate if not found
find_package(OpenGL REQUIRED)

# GLFW - clone (using git submodule) and build
message("Cloning GLFW repository.")
execute_process(
    COMMAND
    git submodule update --init -- libs/glfw
    WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)
message("Building GLFW.")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)

# create executable
add_executable(
    ${CMAKE_PROJECT_NAME}
    src/Chip8/Chip8.c
    src/graphics/GFXscreen.c
    src/utility/utility.c
    src/main.c
    libs/glad/glad.c
)

# set include directories
target_include_directories(
    ${CMAKE_PROJECT_NAME}
    PRIVATE
    libs
    ${OPENGL_INCLUDE_DIR}
    glfw
)

# set library links
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    ${OPENGL_gl_LIBRARY}
    glfw
)
