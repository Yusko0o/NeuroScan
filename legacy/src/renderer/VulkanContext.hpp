#pragma once

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderer.hpp"

#include <vulkan/vulkan.h>

struct GLFWwindow;

class VulkanContext
{
public:
    VulkanContext() = default;
    ~VulkanContext();

    bool initialize(
        GLFWwindow* window
    );

    void drawFrame();

private:
    bool createInstance();

    bool createSurface(
        GLFWwindow* window
    );

    void cleanup();

private:
    VkInstance instance_ =
        VK_NULL_HANDLE;

    VkSurfaceKHR surface_ =
        VK_NULL_HANDLE;

    VulkanDevice device_;
    VulkanSwapchain swapchain_;
    VulkanRenderer renderer_;
};