#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <vector>

struct GLFWwindow;

class VulkanSwapchain
{
public:
    bool initialize(
        GLFWwindow* window,
        VkSurfaceKHR surface,
        VulkanDevice& device
    );

    void cleanup(
        VkDevice device
    );

    VkSwapchainKHR swapchain() const
    {
        return swapchain_;
    }

    VkFormat imageFormat() const
    {
        return imageFormat_;
    }

    VkExtent2D extent() const
    {
        return extent_;
    }

    const std::vector<VkImage>& images() const
    {
        return images_;
    }

    const std::vector<VkImageView>& imageViews() const
    {
        return imageViews_;
    }

private:
    VkSurfaceFormatKHR chooseSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& formats
    );

    VkPresentModeKHR choosePresentMode(
        const std::vector<VkPresentModeKHR>& modes
    );

    VkExtent2D chooseExtent(
        GLFWwindow* window,
        const VkSurfaceCapabilitiesKHR& capabilities
    );

private:
    VkSwapchainKHR swapchain_ =
        VK_NULL_HANDLE;

    VkFormat imageFormat_{};

    VkExtent2D extent_{};

    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;
};