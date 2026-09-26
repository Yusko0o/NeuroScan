#pragma once

#include <vulkan/vulkan.h>

#include <optional>

class VulkanDevice
{
public:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool complete() const
        {
            return graphicsFamily.has_value()
                && presentFamily.has_value();
        }
    };

public:
    bool initialize(
        VkInstance instance,
        VkSurfaceKHR surface
    );

    void cleanup();

    VkPhysicalDevice physicalDevice() const
    {
        return physicalDevice_;
    }

    VkDevice device() const
    {
        return device_;
    }

    VkQueue graphicsQueue() const
    {
        return graphicsQueue_;
    }

    VkQueue presentQueue() const
    {
        return presentQueue_;
    }

    QueueFamilyIndices queueFamilies() const
    {
        return queueFamilies_;
    }

private:
    QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice device,
        VkSurfaceKHR surface
    );

private:
    VkPhysicalDevice physicalDevice_ =
        VK_NULL_HANDLE;

    VkDevice device_ =
        VK_NULL_HANDLE;

    VkQueue graphicsQueue_ =
        VK_NULL_HANDLE;

    VkQueue presentQueue_ =
        VK_NULL_HANDLE;

    QueueFamilyIndices queueFamilies_;
};