cmake_minimum_required (VERSION 3.0.2)

#Add Source Files to our project Files
file(GLOB_RECURSE SCALPEL_RENDERER_SOURCE_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_library(ScalpelRenderer ${SCALPEL_RENDERER_SOURCE_FILES})

#Link With GLFW
target_link_libraries(ScalpelRenderer glfw ${GLFW_LIBRARIES})
#Link With OpenGL
target_link_libraries(ScalpelRenderer ${OPENGL_LIBRARIES})

#Find and Link With Vulkan TODO: fix for 64 bit
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
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        target_link_libraries(ScalpelRenderer $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
    else()
        target_link_libraries(ScalpelRenderer $ENV{VULKAN_SDK}/Lib32/vulkan-1.lib)
    endif()
else()
    target_link_libraries(ScalpelRenderer vulkan)
endif()