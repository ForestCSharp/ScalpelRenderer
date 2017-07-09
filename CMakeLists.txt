cmake_minimum_required (VERSION 3.0.2)
project(Scalpel)

set(LIB_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Libs)

set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Libs/glm)

set(SCALPEL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Include Directories
include_directories(${SCALPEL_SOURCE_DIR} ${GLM_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR})

#GLFW Windowing Framework for OpenGL and Vulkan
add_subdirectory (${LIB_DIRECTORY}/glfw)

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

add_executable(Scalpel ${SCALPEL_SOURCE_FILES} )

#Link With GLFW
target_link_libraries(Scalpel glfw ${GLFW_LIBRARIES})
#Link With OpenGL
target_link_libraries(Scalpel ${OPENGL_LIBRARIES})

#Find and Link With Vulkan
if (WIN32)
    include_directories($ENV{VK_SDK_PATH}/Include
                        $ENV{VULKAN_SDK}/Include)
    
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            link_directories   ($ENV{VK_SDK_PATH}/Bin
                                $ENV{VK_SDK_PATH}/Lib
                                $ENV{VULKAN_SDK}/Bin
                                $ENV{VULKAN_SDK}/Lib)
    else()
            link_directories   ($ENV{VK_SDK_PATH}/Bin32
                                $ENV{VK_SDK_PATH}/Lib32
                                $ENV{VULKAN_SDK}/Bin32
                                $ENV{VULKAN_SDK}/Lib32)
    endif()
else()
    include_directories($ENV{VK_SDK_PATH}/x86_64/include
                        $ENV{VULKAN_SDK}/include
                        $ENV{VULKAN_SDK}/x86_64/include)
    link_directories   ($ENV{VK_SDK_PATH}/x86_64/lib
                        $ENV{VULKAN_SDK}/lib
                        $ENV{VULKAN_SDK}/x86_64/lib)
endif()

if (WIN32)
    target_link_libraries(Scalpel $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
else()
    target_link_libraries(Scalpel vulkan)
endif()