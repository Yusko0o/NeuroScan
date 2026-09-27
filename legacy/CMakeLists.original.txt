cmake_minimum_required(VERSION 3.24)

project(NeuroScan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(assimp REQUIRED)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
)

FetchContent_MakeAvailable(imgui)

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC
    glfw
    Vulkan::Vulkan
)

if(APPLE)
    target_compile_definitions(
        imgui_lib
        PRIVATE
        VK_ENABLE_BETA_EXTENSIONS
    )
endif()

if(NOT Vulkan_GLSLC_EXECUTABLE)
    find_program(
        Vulkan_GLSLC_EXECUTABLE
        NAMES glslc
        REQUIRED
    )
endif()

set(
    SHADER_OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

file(
    MAKE_DIRECTORY
    ${SHADER_OUTPUT_DIR}
)

add_custom_command(
    OUTPUT
        ${SHADER_OUTPUT_DIR}/brain.vert.spv

    COMMAND
        ${Vulkan_GLSLC_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/brain.vert
        -o
        ${SHADER_OUTPUT_DIR}/brain.vert.spv

    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/brain.vert

    VERBATIM
)

add_custom_command(
    OUTPUT
        ${SHADER_OUTPUT_DIR}/brain.frag.spv

    COMMAND
        ${Vulkan_GLSLC_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/brain.frag
        -o
        ${SHADER_OUTPUT_DIR}/brain.frag.spv

    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/brain.frag

    VERBATIM
)

add_custom_target(
    NeuroScanShaders
    DEPENDS
        ${SHADER_OUTPUT_DIR}/brain.vert.spv
        ${SHADER_OUTPUT_DIR}/brain.frag.spv
)

add_executable(
    NeuroScan

    src/main.cpp

    src/core/Application.cpp

    src/renderer/VulkanContext.cpp
    src/renderer/VulkanDevice.cpp
    src/renderer/VulkanSwapchain.cpp
    src/renderer/VulkanRenderer.cpp

    src/renderer/BrainMesh.cpp
    src/renderer/Camera.cpp

    src/ui/NeuroUi.cpp
)

add_dependencies(
    NeuroScan
    NeuroScanShaders
)

target_include_directories(
    NeuroScan
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    NeuroScan
    PRIVATE

    glfw
    glm::glm
    Vulkan::Vulkan
    assimp::assimp
    imgui_lib
)

target_compile_definitions(
    NeuroScan
    PRIVATE

    NEUROSCAN_MODEL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/models/brain.glb"

    NEUROSCAN_VERT_SHADER="${SHADER_OUTPUT_DIR}/brain.vert.spv"

    NEUROSCAN_FRAG_SHADER="${SHADER_OUTPUT_DIR}/brain.frag.spv"
)

if(APPLE)
    target_compile_definitions(
        NeuroScan
        PRIVATE
        VK_ENABLE_BETA_EXTENSIONS
    )
endif()