#pragma once

#include "BrainMesh.hpp"
#include "Camera.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "ui/NeuroUi.hpp"

#include <vulkan/vulkan.h>

#include <vector>

struct GLFWwindow;

class VulkanRenderer
{
public:
    bool initialize(
        GLFWwindow* window,
        VkInstance instance,
        VulkanDevice& device,
        VulkanSwapchain& swapchain
    );

    void drawFrame(
        VulkanDevice& device,
        VulkanSwapchain& swapchain
    );

    void cleanup(
        VkDevice device
    );

private:
    bool createRenderPass(
        VkDevice device,
        VkFormat imageFormat
    );

    bool createDepthResources(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkExtent2D extent
    );

    bool createFramebuffers(
        VkDevice device,
        const VulkanSwapchain& swapchain
    );

    bool createCommandPool(
        VkDevice device,
        uint32_t graphicsQueueFamily
    );

    bool createCommandBuffers(
        VkDevice device
    );

    bool createSyncObjects(
        VkDevice device
    );

    bool createImGuiDescriptorPool(
        VkDevice device
    );

    bool initializeImGui(
        GLFWwindow* window,
        VkInstance instance,
        VulkanDevice& device,
        VulkanSwapchain& swapchain
    );

    void setupImGuiStyle();

    bool recordCommandBuffer(
        VkCommandBuffer commandBuffer,
        uint32_t imageIndex,
        const VulkanSwapchain& swapchain
    );

    VkFormat findDepthFormat(
        VkPhysicalDevice physicalDevice
    );

    uint32_t findMemoryType(
        VkPhysicalDevice physicalDevice,
        uint32_t typeFilter,
        VkMemoryPropertyFlags properties
    );

private:
    VkRenderPass renderPass_ =
        VK_NULL_HANDLE;

    std::vector<VkFramebuffer>
        framebuffers_;

    VkImage depthImage_ =
        VK_NULL_HANDLE;

    VkDeviceMemory depthImageMemory_ =
        VK_NULL_HANDLE;

    VkImageView depthImageView_ =
        VK_NULL_HANDLE;

    VkFormat depthFormat_ =
        VK_FORMAT_UNDEFINED;

    VkCommandPool commandPool_ =
        VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer_ =
        VK_NULL_HANDLE;

    VkSemaphore imageAvailableSemaphore_ =
        VK_NULL_HANDLE;

    VkSemaphore renderFinishedSemaphore_ =
        VK_NULL_HANDLE;

    VkFence inFlightFence_ =
        VK_NULL_HANDLE;

    VkDescriptorPool imguiDescriptorPool_ =
        VK_NULL_HANDLE;

    bool imguiInitialized_ =
        false;

    NeuroUI ui_;

    BrainMesh brainMesh_;
    Camera camera_;

    bool brainInitialized_ =
        false;
};