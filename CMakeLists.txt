cmake_minimum_required(VERSION 3.15)
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

include_directories(/usr/local/include)

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

set(BUILD_SHARED_LIBS  ON CACHE BOOL "" FORCE)

add_subdirectory(vulkan-renderer/third-party/assimp)
include_directories(vulkan-renderer/third-party/assimp/include/assimp)

add_compile_definitions(STB_IMAGE_WRITE_IMPLEMENTATION)

add_executable(vulkan_renderer ${SOURCES} ${IMGUI_SRC})

if (APPLE)
        target_link_libraries(vulkan_renderer glfw ${GLFW_LIBRARIES} /usr/local/lib/libshaderc_shared.1.dylib assimp /usr/local/lib/libvulkan.1.dylib)
else()
        target_link_libraries(vulkan_renderer glfw ${GLFW_LIBRARIES} assimp $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)


        set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
        message(STATUS ${RUNTIME_OUTPUT_DIRECTORY})
        message(STATUS ${EXECUTABLE_OUTPUT_PATH})
        message(STATUS ${CMAKE_BINARY_DIR})
        message(STATUS ${CMAKE_BUILD_TYPE})

        add_custom_command(TARGET vulkan_renderer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "vulkan-renderer/third-party/glfw/src/$<CONFIG>/glfw3$<$<CONFIG:Debug>:d>.dll"
                ${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>)
        
        message(STATUS MSVC_VERSION: ${MSVC_VERSION})
        IF(MSVC_VERSION EQUAL 1800)
                SET(ASSIMP_MSVC_VERSION "vc120")
        ELSEIF(MSVC_VERSION EQUAL 1900)
                SET(ASSIMP_MSVC_VERSION "vc140")
        ELSEIF((MSVC_VERSION GREATER_EQUAL 1910) OR (MSVC_VERSION LESS_EQUAL 1919))
                SET(ASSIMP_MSVC_VERSION "vc141")
        ENDIF()
        add_custom_command(TARGET vulkan_renderer POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "vulkan-renderer/third-party/assimp/bin/$<CONFIG>/assimp-${ASSIMP_MSVC_VERSION}-mt$<$<CONFIG:Debug>:d>.dll"      # <--this is in-file
                ${EXECUTABLE_OUTPUT_PATH}/$<CONFIG>)
endif()