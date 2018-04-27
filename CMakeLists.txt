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

add_subdirectory(${LIB_DIRECTORY}/glfw)

set(SHADERC_SKIP_TESTS TRUE)
add_subdirectory(${LIB_DIRECTORY}/shaderc)

#Include Directories
include_directories(${SCALPEL_SOURCE_DIR} ${LIB_DIRECTORY} ${GLM_INCLUDE_DIR} ${STB_INCLUDE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Find OpenGL
find_package(OpenGL REQUIRED)

add_executable(Scalpel ${CMAKE_CURRENT_SOURCE_DIR}/Src/main.cpp)


#Asset directory
target_compile_definitions(Scalpel PRIVATE ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Assets")

#Link With GLFW
target_link_libraries(Scalpel PUBLIC glfw ${GLFW_LIBRARIES})
#Link With OpenGL
target_link_libraries(Scalpel PUBLIC ${OPENGL_LIBRARIES})

#Renderer Library
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/Src/Renderer)
target_link_libraries(Scalpel PUBLIC ScalpelRenderer)

#ShaderC
target_link_libraries(Scalpel PRIVATE shaderc)

#Find and Include Vulkan
if (WIN32)
    include_directories($ENV{VK_SDK_PATH}/Include
                        $ENV{VULKAN_SDK}/Include)
else()
    include_directories($ENV{VK_SDK_PATH}/x86_64/include
                        $ENV{VULKAN_SDK}/include
                        $ENV{VULKAN_SDK}/x86_64/include)
endif()

