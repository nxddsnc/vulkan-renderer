cmake_minimum_required(VERSION 3.17)
project(vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)

include_directories(vulkan-renderer/third-party/glfw/include)
include_directories(vulkan-renderer/third-party/glm)

include_directories($ENV{VULKAN_SDK}/include)
include_directories(vulkan-renderer)
include_directories(vulkan-renderer/Vulkan)
include_directories(vulkan-renderer/Camera)
include_directories(vulkan-renderer/Debug)
include_directories(vulkan-renderer/PostEffect)
include_directories(vulkan-renderer/RenderScene)
include_directories(vulkan-renderer/Shaders)
include_directories(vulkan-renderer/ShadowMap)
include_directories(vulkan-renderer/third-party/includes)
include_directories(vulkan-renderer/third-party/imgui)


add_subdirectory(vulkan-renderer/third-party/glfw)

file(GLOB SOURCES vulkan-renderer/*.*
        vulkan-renderer/Vulkan/*.cpp
        vulkan-renderer/Camera/*.cpp
        vulkan-renderer/Debug/*.cpp
        vulkan-renderer/PostEffect/*.cpp
        vulkan-renderer/RenderScene/*.cpp
        vulkan-renderer/Shaders/*.cpp
        vulkan-renderer/ShadowMap/*.cpp)
file(GLOB IMGUI_SRC vulkan-renderer/third-party/imgui/*.cpp)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(vulkan-renderer/third-party/shaderc)
include_directories(vulkan-renderer/third-party/shaderc/libshaderc/include)

add_subdirectory(vulkan-renderer/third-party/assimp)
include_directories(vulkan-renderer/third-party/assimp/include/assimp)

add_executable(vulkan_renderer ${SOURCES} ${IMGUI_SRC})
target_link_libraries(vulkan_renderer glfw ${GLFW_LIBRARIES} shaderc assimp $ENV{DYLD_LIBRARY_PATH})
