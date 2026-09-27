cmake_minimum_required(VERSION 3.24)
project(NeuroScan VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick QuickControls2)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
if(NOT Vulkan_GLSLC_EXECUTABLE)
    find_program(Vulkan_GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)
endif()
set(SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY "${SHADER_DIR}")
foreach(stage vert frag)
    add_custom_command(OUTPUT "${SHADER_DIR}/brain.${stage}.spv"
        COMMAND "${Vulkan_GLSLC_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/brain.${stage}"
                -o "${SHADER_DIR}/brain.${stage}.spv"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/brain.${stage}" VERBATIM)
endforeach()
add_custom_target(NeuroScanShaders DEPENDS "${SHADER_DIR}/brain.vert.spv" "${SHADER_DIR}/brain.frag.spv")
qt_add_executable(NeuroScan src/main.cpp src/qt/BrainViewport.cpp src/qt/BrainViewport.hpp
    src/qt/BrainTextureNode.cpp src/qt/BrainTextureNode.hpp
    src/renderer/BrainMesh.cpp src/renderer/Camera.cpp)
qt_add_resources(NeuroScan "ui" PREFIX "/" FILES qml/Main.qml)
add_dependencies(NeuroScan NeuroScanShaders)
target_include_directories(NeuroScan PRIVATE src)
target_link_libraries(NeuroScan PRIVATE Qt6::Quick Qt6::QuickControls2 Vulkan::Vulkan glm::glm assimp::assimp)
target_compile_definitions(NeuroScan PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE NOMINMAX)
if(MSVC)
    target_compile_options(NeuroScan PRIVATE /W4 /utf-8)
else()
    target_compile_options(NeuroScan PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
endif()
# Keep runtime data beside the executable, independent of the working directory.
add_custom_target(NeuroScanRuntime ALL
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:NeuroScan>/assets/models"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:NeuroScan>/shaders"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/assets/models/brain.glb" "$<TARGET_FILE_DIR:NeuroScan>/assets/models/brain.glb"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SHADER_DIR}/brain.vert.spv" "${SHADER_DIR}/brain.frag.spv" "$<TARGET_FILE_DIR:NeuroScan>/shaders"
    DEPENDS NeuroScan NeuroScanShaders VERBATIM)
include(CTest)
if(BUILD_TESTING)
    add_executable(CameraTest tests/CameraTest.cpp src/renderer/Camera.cpp)
    target_include_directories(CameraTest PRIVATE src)
    target_link_libraries(CameraTest PRIVATE glm::glm)
    target_compile_definitions(CameraTest PRIVATE GLM_FORCE_DEPTH_ZERO_TO_ONE)
    add_test(NAME CameraMath COMMAND CameraTest)
endif()
