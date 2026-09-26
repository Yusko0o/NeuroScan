#include "VulkanContext.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

VulkanContext::~VulkanContext()
{
    cleanup();
}

bool VulkanContext::initialize(
    GLFWwindow* window
)
{
    if (!createInstance())
    {
        return false;
    }

    if (!createSurface(window))
    {
        return false;
    }

    if (
        !device_.initialize(
            instance_,
            surface_
        )
    )
    {
        return false;
    }

    if (
        !swapchain_.initialize(
            window,
            surface_,
            device_
        )
    )
    {
        return false;
    }

    if (
        !renderer_.initialize(
            window,
            instance_,
            device_,
            swapchain_
        )
    )
    {
        return false;
    }

    std::cout
        << "Vulkan core initialized.\n";

    return true;
}

bool VulkanContext::createInstance()
{
    VkApplicationInfo appInfo{};

    appInfo.sType =
        VK_STRUCTURE_TYPE_APPLICATION_INFO;

    appInfo.pApplicationName =
        "NeuroScan";

    appInfo.applicationVersion =
        VK_MAKE_VERSION(0, 1, 0);

    appInfo.pEngineName =
        "NeuroScan Engine";

    appInfo.engineVersion =
        VK_MAKE_VERSION(0, 1, 0);

    appInfo.apiVersion =
        VK_API_VERSION_1_2;

    uint32_t extensionCount = 0;

    const char** extensions =
        glfwGetRequiredInstanceExtensions(
            &extensionCount
        );

    if (
        extensions == nullptr ||
        extensionCount == 0
    )
    {
        std::cerr
            << "GLFW could not provide Vulkan extensions.\n";

        return false;
    }

    std::vector<const char*>
        requiredExtensions(
            extensions,
            extensions + extensionCount
        );

#ifdef __APPLE__

    requiredExtensions.push_back(
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    );

#endif

    VkInstanceCreateInfo createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    createInfo.pApplicationInfo =
        &appInfo;

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(
            requiredExtensions.size()
        );

    createInfo.ppEnabledExtensionNames =
        requiredExtensions.data();

#ifdef __APPLE__

    createInfo.flags |=
        VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

#endif

    if (
        vkCreateInstance(
            &createInfo,
            nullptr,
            &instance_
        )
        != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create Vulkan instance.\n";

        return false;
    }

    std::cout
        << "Vulkan instance ready.\n";

    return true;
}

bool VulkanContext::createSurface(
    GLFWwindow* window
)
{
    if (
        glfwCreateWindowSurface(
            instance_,
            window,
            nullptr,
            &surface_
        )
        != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create Vulkan surface.\n";

        return false;
    }

    std::cout
        << "Vulkan surface ready.\n";

    return true;
}

void VulkanContext::drawFrame()
{
    renderer_.drawFrame(
        device_,
        swapchain_
    );
}

void VulkanContext::cleanup()
{
    if (
        device_.device()
        != VK_NULL_HANDLE
    )
    {
        vkDeviceWaitIdle(
            device_.device()
        );

        renderer_.cleanup(
            device_.device()
        );

        swapchain_.cleanup(
            device_.device()
        );
    }

    device_.cleanup();

    if (
        surface_
        != VK_NULL_HANDLE
    )
    {
        vkDestroySurfaceKHR(
            instance_,
            surface_,
            nullptr
        );

        surface_ =
            VK_NULL_HANDLE;
    }

    if (
        instance_
        != VK_NULL_HANDLE
    )
    {
        vkDestroyInstance(
            instance_,
            nullptr
        );

        instance_ =
            VK_NULL_HANDLE;
    }
}