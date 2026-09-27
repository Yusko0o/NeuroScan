#include "Application.hpp"
#include "renderer/VulkanContext.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

int Application::run()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return 1;
    }

    if (!glfwVulkanSupported())
    {
        std::cerr << "Vulkan is not supported by GLFW.\n";

        glfwTerminate();
        return 1;
    }

    glfwWindowHint(
        GLFW_CLIENT_API,
        GLFW_NO_API
    );

    glfwWindowHint(
        GLFW_RESIZABLE,
        GLFW_TRUE
    );

    GLFWwindow* window = glfwCreateWindow(
        1600,
        950,
        "NEUROSCAN",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window.\n";

        glfwTerminate();
        return 1;
    }

    {
        VulkanContext vulkan;

        if (!vulkan.initialize(window))
        {
            std::cerr << "Failed to initialize Vulkan.\n";

            glfwDestroyWindow(window);
            glfwTerminate();

            return 1;
        }

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            vulkan.drawFrame();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}