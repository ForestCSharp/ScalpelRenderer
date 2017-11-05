cmake_minimum_required (VERSION 3.0.2)
project(Scalpel)

set(LIB_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Libs)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Libs/glm)

set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Libs/stb)

set(SCALPEL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#GLFW Windowing Framework for OpenGL and Vulkan
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory (${LIB_DIRECTORY}/glfw)

#Include Directories
include_directories(${SCALPEL_SOURCE_DIR} ${GLM_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR})

#Add Source Files to our project Files
file(GLOB_RECURSE SCALPEL_SOURCE_FILES
      ${SCALPEL_SOURCE_DIR}/*.h
      ${SCALPEL_SOURCE_DIR}/*.cpp
      ${SCALPEL_SOURCE_DIR}/*.c
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Find OpenGL
find_package(OpenGL REQUIRED)

add_executable(Scalpel ${CMAKE_CURRENT_SOURCE_DIR}/Src/main.cpp)

#Link With GLFW
target_link_libraries(Scalpel glfw ${GLFW_LIBRARIES})
#Link With OpenGL
target_link_libraries(Scalpel ${OPENGL_LIBRARIES})

#Renderer Library
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/Src/Renderer)
target_link_libraries(Scalpel ScalpelRenderer)