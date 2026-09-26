#pragma once

#include "Camera.hpp"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

class BrainMesh
{
public:
    struct Viewport
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct RenderSettings
    {
        float opacity = 0.82f;
        float brightness = 0.74f;
        float activityEnabled = 1.0f;
        float time = 0.0f;
    };

public:
    bool initialize(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkRenderPass renderPass,
        const std::string& modelPath,
        const std::string& vertexShaderPath,
        const std::string& fragmentShaderPath
    );

    void draw(
        VkCommandBuffer commandBuffer,
        const Camera& camera,
        const Viewport& viewport,
        const RenderSettings& settings
    );

    void cleanup(
        VkDevice device
    );

private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
    };

    struct PushConstants
    {
        glm::mat4 mvp;
        glm::vec4 renderSettings;
    };

private:
    bool loadModel(
        const std::string& path
    );

    bool createVertexBuffer(
        VkPhysicalDevice physicalDevice,
        VkDevice device
    );

    bool createIndexBuffer(
        VkPhysicalDevice physicalDevice,
        VkDevice device
    );

    bool createGraphicsPipeline(
        VkDevice device,
        VkRenderPass renderPass,
        const std::string& vertexShaderPath,
        const std::string& fragmentShaderPath
    );

    bool createBuffer(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& memory
    );

    uint32_t findMemoryType(
        VkPhysicalDevice physicalDevice,
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties
    );

    std::vector<char> readFile(
        const std::string& path
    );

    VkShaderModule createShaderModule(
        VkDevice device,
        const std::vector<char>& code
    );

private:
    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;

    VkBuffer vertexBuffer_ =
        VK_NULL_HANDLE;

    VkDeviceMemory vertexMemory_ =
        VK_NULL_HANDLE;

    VkBuffer indexBuffer_ =
        VK_NULL_HANDLE;

    VkDeviceMemory indexMemory_ =
        VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout_ =
        VK_NULL_HANDLE;

    VkPipeline pipeline_ =
        VK_NULL_HANDLE;
};