#include "VulkanSwapchain.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

bool VulkanSwapchain::initialize(
    GLFWwindow* window,
    VkSurfaceKHR surface,
    VulkanDevice& device
)
{
    VkSurfaceCapabilitiesKHR capabilities{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device.physicalDevice(),
        surface,
        &capabilities
    );

    uint32_t formatCount = 0;

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device.physicalDevice(),
        surface,
        &formatCount,
        nullptr
    );

    std::vector<VkSurfaceFormatKHR>
        formats(formatCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        device.physicalDevice(),
        surface,
        &formatCount,
        formats.data()
    );

    uint32_t presentModeCount = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device.physicalDevice(),
        surface,
        &presentModeCount,
        nullptr
    );

    std::vector<VkPresentModeKHR>
        presentModes(presentModeCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device.physicalDevice(),
        surface,
        &presentModeCount,
        presentModes.data()
    );

    VkSurfaceFormatKHR surfaceFormat =
        chooseSurfaceFormat(formats);

    VkPresentModeKHR presentMode =
        choosePresentMode(presentModes);

    VkExtent2D extent =
        chooseExtent(
            window,
            capabilities
        );

    uint32_t imageCount =
        capabilities.minImageCount + 1;

    if (
        capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount
    )
    {
        imageCount =
            capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    createInfo.surface =
        surface;

    createInfo.minImageCount =
        imageCount;

    createInfo.imageFormat =
        surfaceFormat.format;

    createInfo.imageColorSpace =
        surfaceFormat.colorSpace;

    createInfo.imageExtent =
        extent;

    createInfo.imageArrayLayers =
        1;

    createInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices =
        device.queueFamilies();

    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (
        indices.graphicsFamily
        != indices.presentFamily
    )
    {
        createInfo.imageSharingMode =
            VK_SHARING_MODE_CONCURRENT;

        createInfo.queueFamilyIndexCount =
            2;

        createInfo.pQueueFamilyIndices =
            queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode =
            VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform =
        capabilities.currentTransform;

    createInfo.compositeAlpha =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode =
        presentMode;

    createInfo.clipped =
        VK_TRUE;

    VkResult result =
        vkCreateSwapchainKHR(
            device.device(),
            &createInfo,
            nullptr,
            &swapchain_
        );

    if (result != VK_SUCCESS)
    {
        std::cerr
            << "Failed to create swapchain.\n";

        return false;
    }

    vkGetSwapchainImagesKHR(
        device.device(),
        swapchain_,
        &imageCount,
        nullptr
    );

    images_.resize(imageCount);

    vkGetSwapchainImagesKHR(
        device.device(),
        swapchain_,
        &imageCount,
        images_.data()
    );

    imageFormat_ =
        surfaceFormat.format;

    extent_ =
        extent;

    imageViews_.resize(
        images_.size()
    );

    for (
        size_t i = 0;
        i < images_.size();
        ++i
    )
    {
        VkImageViewCreateInfo viewInfo{};

        viewInfo.sType =
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        viewInfo.image =
            images_[i];

        viewInfo.viewType =
            VK_IMAGE_VIEW_TYPE_2D;

        viewInfo.format =
            imageFormat_;

        viewInfo.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;

        viewInfo.subresourceRange.baseMipLevel =
            0;

        viewInfo.subresourceRange.levelCount =
            1;

        viewInfo.subresourceRange.baseArrayLayer =
            0;

        viewInfo.subresourceRange.layerCount =
            1;

        if (
            vkCreateImageView(
                device.device(),
                &viewInfo,
                nullptr,
                &imageViews_[i]
            )
            != VK_SUCCESS
        )
        {
            std::cerr
                << "Failed to create image view.\n";

            return false;
        }
    }

    std::cout
        << "Swapchain ready with "
        << images_.size()
        << " images.\n";

    return true;
}

VkSurfaceFormatKHR
VulkanSwapchain::chooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats
)
{
    for (const auto& format : formats)
    {
        if (
            format.format
                == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace
                == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        )
        {
            return format;
        }
    }

    return formats.front();
}

VkPresentModeKHR
VulkanSwapchain::choosePresentMode(
    const std::vector<VkPresentModeKHR>& modes
)
{
    for (const auto mode : modes)
    {
        if (
            mode == VK_PRESENT_MODE_MAILBOX_KHR
        )
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanSwapchain::chooseExtent(
    GLFWwindow* window,
    const VkSurfaceCapabilitiesKHR& capabilities
)
{
    if (
        capabilities.currentExtent.width
        != UINT32_MAX
    )
    {
        return capabilities.currentExtent;
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(
        window,
        &width,
        &height
    );

    VkExtent2D extent{
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    extent.width =
        std::clamp(
            extent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );

    extent.height =
        std::clamp(
            extent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );

    return extent;
}

void VulkanSwapchain::cleanup(
    VkDevice device
)
{
    for (VkImageView view : imageViews_)
    {
        vkDestroyImageView(
            device,
            view,
            nullptr
        );
    }

    imageViews_.clear();

    if (swapchain_ != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(
            device,
            swapchain_,
            nullptr
        );

        swapchain_ =
            VK_NULL_HANDLE;
    }
}