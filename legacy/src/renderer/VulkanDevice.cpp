#include "VulkanDevice.hpp"

#include <iostream>
#include <set>
#include <vector>

bool VulkanDevice::initialize(
    VkInstance instance,
    VkSurfaceKHR surface
)
{
    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices(
        instance,
        &deviceCount,
        nullptr
    );

    if (deviceCount == 0)
    {
        std::cerr
            << "No Vulkan-compatible GPU found.\n";

        return false;
    }

    std::vector<VkPhysicalDevice>
        devices(deviceCount);

    vkEnumeratePhysicalDevices(
        instance,
        &deviceCount,
        devices.data()
    );

    for (VkPhysicalDevice candidate : devices)
    {
        QueueFamilyIndices indices =
            findQueueFamilies(
                candidate,
                surface
            );

        if (!indices.complete())
        {
            continue;
        }

        VkPhysicalDeviceProperties properties{};

        vkGetPhysicalDeviceProperties(
            candidate,
            &properties
        );

        physicalDevice_ = candidate;
        queueFamilies_ = indices;

        std::cout
            << "Selected GPU: "
            << properties.deviceName
            << '\n';

        break;
    }

    if (physicalDevice_ == VK_NULL_HANDLE)
    {
        std::cerr
            << "No suitable Vulkan GPU found.\n";

        return false;
    }

    std::set<uint32_t> uniqueFamilies{
        queueFamilies_.graphicsFamily.value(),
        queueFamilies_.presentFamily.value()
    };

    std::vector<VkDeviceQueueCreateInfo>
        queueInfos;

    float queuePriority = 1.0f;

    for (uint32_t family : uniqueFamilies)
    {
        VkDeviceQueueCreateInfo info{};

        info.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        info.queueFamilyIndex =
            family;

        info.queueCount =
            1;

        info.pQueuePriorities =
            &queuePriority;

        queueInfos.push_back(info);
    }

    VkPhysicalDeviceFeatures features{};

    std::vector<const char*> extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef __APPLE__
    extensions.push_back(
        "VK_KHR_portability_subset"
    );
#endif

    VkDeviceCreateInfo createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(
            queueInfos.size()
        );

    createInfo.pQueueCreateInfos =
        queueInfos.data();

    createInfo.pEnabledFeatures =
        &features;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(
            extensions.size()
        );

    createInfo.ppEnabledExtensionNames =
        extensions.data();

    VkResult result =
        vkCreateDevice(
            physicalDevice_,
            &createInfo,
            nullptr,
            &device_
        );

    if (result != VK_SUCCESS)
    {
        std::cerr
            << "Failed to create logical device.\n";

        return false;
    }

    vkGetDeviceQueue(
        device_,
        queueFamilies_.graphicsFamily.value(),
        0,
        &graphicsQueue_
    );

    vkGetDeviceQueue(
        device_,
        queueFamilies_.presentFamily.value(),
        0,
        &presentQueue_
    );

    std::cout
        << "Logical device ready.\n";

    return true;
}

VulkanDevice::QueueFamilyIndices
VulkanDevice::findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR surface
)
{
    QueueFamilyIndices indices;

    uint32_t count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &count,
        nullptr
    );

    std::vector<VkQueueFamilyProperties>
        families(count);

    vkGetPhysicalDeviceQueueFamilyProperties(
        device,
        &count,
        families.data()
    );

    for (uint32_t i = 0; i < count; ++i)
    {
        if (
            families[i].queueFlags
            & VK_QUEUE_GRAPHICS_BIT
        )
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport =
            VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &presentSupport
        );

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.complete())
        {
            break;
        }
    }

    return indices;
}

void VulkanDevice::cleanup()
{
    if (device_ != VK_NULL_HANDLE)
    {
        vkDestroyDevice(
            device_,
            nullptr
        );

        device_ =
            VK_NULL_HANDLE;
    }

    physicalDevice_ =
        VK_NULL_HANDLE;
}