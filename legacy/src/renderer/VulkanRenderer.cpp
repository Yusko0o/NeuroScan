#include "VulkanRenderer.hpp"

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>

bool VulkanRenderer::initialize(
    GLFWwindow* window,
    VkInstance instance,
    VulkanDevice& device,
    VulkanSwapchain& swapchain
)
{
    depthFormat_ =
        findDepthFormat(
            device.physicalDevice()
        );

    if (
        !createRenderPass(
            device.device(),
            swapchain.imageFormat()
        )
    )
    {
        return false;
    }

    if (
        !createDepthResources(
            device.physicalDevice(),
            device.device(),
            swapchain.extent()
        )
    )
    {
        return false;
    }

    if (
        !createFramebuffers(
            device.device(),
            swapchain
        )
    )
    {
        return false;
    }

    const auto indices =
        device.queueFamilies();

    if (
        !createCommandPool(
            device.device(),
            indices.graphicsFamily.value()
        )
    )
    {
        return false;
    }

    if (
        !createCommandBuffers(
            device.device()
        )
    )
    {
        return false;
    }

    if (
        !createSyncObjects(
            device.device()
        )
    )
    {
        return false;
    }

    if (
        !createImGuiDescriptorPool(
            device.device()
        )
    )
    {
        return false;
    }

    if (
        !initializeImGui(
            window,
            instance,
            device,
            swapchain
        )
    )
    {
        return false;
    }

    if (
        !brainMesh_.initialize(
            device.physicalDevice(),
            device.device(),
            renderPass_,
            NEUROSCAN_MODEL_PATH,
            NEUROSCAN_VERT_SHADER,
            NEUROSCAN_FRAG_SHADER
        )
    )
    {
        std::cerr
            << "Failed to initialize brain renderer.\n";

        return false;
    }

    brainInitialized_ =
        true;

    std::cout
        << "Vulkan renderer + real 3D brain ready.\n";

    return true;
}

VkFormat VulkanRenderer::findDepthFormat(
    VkPhysicalDevice physicalDevice
)
{
    const std::array<
        VkFormat,
        3
    > candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    for (
        VkFormat format :
        candidates
    )
    {
        VkFormatProperties properties{};

        vkGetPhysicalDeviceFormatProperties(
            physicalDevice,
            format,
            &properties
        );

        if (
            properties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        )
        {
            return format;
        }
    }

    throw std::runtime_error(
        "No supported Vulkan depth format."
    );
}

uint32_t VulkanRenderer::findMemoryType(
    VkPhysicalDevice physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties
)
{
    VkPhysicalDeviceMemoryProperties
        memoryProperties{};

    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice,
        &memoryProperties
    );

    for (
        uint32_t i = 0;
        i < memoryProperties.memoryTypeCount;
        ++i
    )
    {
        if (
            (typeFilter & (1u << i)) &&
            (
                memoryProperties
                    .memoryTypes[i]
                    .propertyFlags &
                properties
            ) == properties
        )
        {
            return i;
        }
    }

    throw std::runtime_error(
        "No suitable Vulkan memory type."
    );
}

bool VulkanRenderer::createDepthResources(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkExtent2D extent
)
{
    VkImageCreateInfo imageInfo{};

    imageInfo.sType =
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    imageInfo.imageType =
        VK_IMAGE_TYPE_2D;

    imageInfo.extent.width =
        extent.width;

    imageInfo.extent.height =
        extent.height;

    imageInfo.extent.depth =
        1;

    imageInfo.mipLevels =
        1;

    imageInfo.arrayLayers =
        1;

    imageInfo.format =
        depthFormat_;

    imageInfo.tiling =
        VK_IMAGE_TILING_OPTIMAL;

    imageInfo.initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.usage =
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    imageInfo.samples =
        VK_SAMPLE_COUNT_1_BIT;

    imageInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;

    if (
        vkCreateImage(
            device,
            &imageInfo,
            nullptr,
            &depthImage_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create depth image.\n";

        return false;
    }

    VkMemoryRequirements
        memoryRequirements{};

    vkGetImageMemoryRequirements(
        device,
        depthImage_,
        &memoryRequirements
    );

    VkMemoryAllocateInfo
        allocationInfo{};

    allocationInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocationInfo.allocationSize =
        memoryRequirements.size;

    allocationInfo.memoryTypeIndex =
        findMemoryType(
            physicalDevice,
            memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

    if (
        vkAllocateMemory(
            device,
            &allocationInfo,
            nullptr,
            &depthImageMemory_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to allocate depth memory.\n";

        return false;
    }

    if (
        vkBindImageMemory(
            device,
            depthImage_,
            depthImageMemory_,
            0
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to bind depth image memory.\n";

        return false;
    }

    VkImageViewCreateInfo viewInfo{};

    viewInfo.sType =
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    viewInfo.image =
        depthImage_;

    viewInfo.viewType =
        VK_IMAGE_VIEW_TYPE_2D;

    viewInfo.format =
        depthFormat_;

    viewInfo.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT;

    if (
        depthFormat_ ==
            VK_FORMAT_D32_SFLOAT_S8_UINT ||
        depthFormat_ ==
            VK_FORMAT_D24_UNORM_S8_UINT
    )
    {
        viewInfo.subresourceRange.aspectMask |=
            VK_IMAGE_ASPECT_STENCIL_BIT;
    }

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
            device,
            &viewInfo,
            nullptr,
            &depthImageView_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create depth image view.\n";

        return false;
    }

    return true;
}

bool VulkanRenderer::createRenderPass(
    VkDevice device,
    VkFormat imageFormat
)
{
    VkAttachmentDescription
        colorAttachment{};

    colorAttachment.format =
        imageFormat;

    colorAttachment.samples =
        VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;

    colorAttachment.storeOp =
        VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    colorAttachment.stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;

    colorAttachment.finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference
        colorReference{};

    colorReference.attachment =
        0;

    colorReference.layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription
        depthAttachment{};

    depthAttachment.format =
        depthFormat_;

    depthAttachment.samples =
        VK_SAMPLE_COUNT_1_BIT;

    depthAttachment.loadOp =
        VK_ATTACHMENT_LOAD_OP_CLEAR;

    depthAttachment.storeOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;

    depthAttachment.stencilLoadOp =
        VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    depthAttachment.stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;

    depthAttachment.initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED;

    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference
        depthReference{};

    depthReference.attachment =
        1;

    depthReference.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription
        subpass{};

    subpass.pipelineBindPoint =
        VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount =
        1;

    subpass.pColorAttachments =
        &colorReference;

    subpass.pDepthStencilAttachment =
        &depthReference;

    VkSubpassDependency
        dependency{};

    dependency.srcSubpass =
        VK_SUBPASS_EXTERNAL;

    dependency.dstSubpass =
        0;

    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.srcAccessMask =
        0;

    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const std::array<
        VkAttachmentDescription,
        2
    > attachments = {
        colorAttachment,
        depthAttachment
    };

    VkRenderPassCreateInfo
        createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    createInfo.attachmentCount =
        static_cast<uint32_t>(
            attachments.size()
        );

    createInfo.pAttachments =
        attachments.data();

    createInfo.subpassCount =
        1;

    createInfo.pSubpasses =
        &subpass;

    createInfo.dependencyCount =
        1;

    createInfo.pDependencies =
        &dependency;

    if (
        vkCreateRenderPass(
            device,
            &createInfo,
            nullptr,
            &renderPass_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create render pass.\n";

        return false;
    }

    return true;
}

bool VulkanRenderer::createFramebuffers(
    VkDevice device,
    const VulkanSwapchain& swapchain
)
{
    const auto& imageViews =
        swapchain.imageViews();

    framebuffers_.resize(
        imageViews.size()
    );

    for (
        size_t i = 0;
        i < imageViews.size();
        ++i
    )
    {
        const std::array<
            VkImageView,
            2
        > attachments = {
            imageViews[i],
            depthImageView_
        };

        VkFramebufferCreateInfo
            createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        createInfo.renderPass =
            renderPass_;

        createInfo.attachmentCount =
            static_cast<uint32_t>(
                attachments.size()
            );

        createInfo.pAttachments =
            attachments.data();

        createInfo.width =
            swapchain.extent().width;

        createInfo.height =
            swapchain.extent().height;

        createInfo.layers =
            1;

        if (
            vkCreateFramebuffer(
                device,
                &createInfo,
                nullptr,
                &framebuffers_[i]
            ) != VK_SUCCESS
        )
        {
            std::cerr
                << "Failed to create framebuffer.\n";

            return false;
        }
    }

    return true;
}

bool VulkanRenderer::createCommandPool(
    VkDevice device,
    uint32_t graphicsQueueFamily
)
{
    VkCommandPoolCreateInfo
        createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    createInfo.flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    createInfo.queueFamilyIndex =
        graphicsQueueFamily;

    if (
        vkCreateCommandPool(
            device,
            &createInfo,
            nullptr,
            &commandPool_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create command pool.\n";

        return false;
    }

    return true;
}

bool VulkanRenderer::createCommandBuffers(
    VkDevice device
)
{
    VkCommandBufferAllocateInfo
        info{};

    info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    info.commandPool =
        commandPool_;

    info.level =
        VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    info.commandBufferCount =
        1;

    if (
        vkAllocateCommandBuffers(
            device,
            &info,
            &commandBuffer_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to allocate command buffer.\n";

        return false;
    }

    return true;
}

bool VulkanRenderer::createSyncObjects(
    VkDevice device
)
{
    VkSemaphoreCreateInfo
        semaphoreInfo{};

    semaphoreInfo.sType =
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo
        fenceInfo{};

    fenceInfo.sType =
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    fenceInfo.flags =
        VK_FENCE_CREATE_SIGNALED_BIT;

    if (
        vkCreateSemaphore(
            device,
            &semaphoreInfo,
            nullptr,
            &imageAvailableSemaphore_
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    if (
        vkCreateSemaphore(
            device,
            &semaphoreInfo,
            nullptr,
            &renderFinishedSemaphore_
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    if (
        vkCreateFence(
            device,
            &fenceInfo,
            nullptr,
            &inFlightFence_
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    return true;
}

bool VulkanRenderer::createImGuiDescriptorPool(
    VkDevice device
)
{
    std::array<
        VkDescriptorPoolSize,
        11
    > poolSizes{{
        {
            VK_DESCRIPTOR_TYPE_SAMPLER,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            1000
        },
        {
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            1000
        }
    }};

    VkDescriptorPoolCreateInfo
        poolInfo{};

    poolInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    poolInfo.maxSets =
        1000;

    poolInfo.poolSizeCount =
        static_cast<uint32_t>(
            poolSizes.size()
        );

    poolInfo.pPoolSizes =
        poolSizes.data();

    if (
        vkCreateDescriptorPool(
            device,
            &poolInfo,
            nullptr,
            &imguiDescriptorPool_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to create ImGui descriptor pool.\n";

        return false;
    }

    return true;
}

bool VulkanRenderer::initializeImGui(
    GLFWwindow* window,
    VkInstance instance,
    VulkanDevice& device,
    VulkanSwapchain& swapchain
)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io =
        ImGui::GetIO();

    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;

    setupImGuiStyle();

    if (
        !ImGui_ImplGlfw_InitForVulkan(
            window,
            true
        )
    )
    {
        std::cerr
            << "Failed to initialize ImGui GLFW backend.\n";

        return false;
    }

    ImGui_ImplVulkan_InitInfo
        initInfo{};

    initInfo.Instance =
        instance;

    initInfo.PhysicalDevice =
        device.physicalDevice();

    initInfo.Device =
        device.device();

    initInfo.QueueFamily =
        device.queueFamilies()
            .graphicsFamily
            .value();

    initInfo.Queue =
        device.graphicsQueue();

    initInfo.DescriptorPool =
        imguiDescriptorPool_;

    initInfo.RenderPass =
        renderPass_;

    initInfo.MinImageCount =
        static_cast<uint32_t>(
            swapchain.images().size()
        );

    initInfo.ImageCount =
        static_cast<uint32_t>(
            swapchain.images().size()
        );

    initInfo.MSAASamples =
        VK_SAMPLE_COUNT_1_BIT;

    if (
        !ImGui_ImplVulkan_Init(
            &initInfo
        )
    )
    {
        std::cerr
            << "Failed to initialize ImGui Vulkan backend.\n";

        return false;
    }

    imguiInitialized_ =
        true;

    return true;
}

void VulkanRenderer::setupImGuiStyle()
{
    ImGuiStyle& style =
        ImGui::GetStyle();

    style.WindowRounding =
        3.0f;

    style.ChildRounding =
        3.0f;

    style.FrameRounding =
        3.0f;

    style.WindowBorderSize =
        1.0f;

    style.ChildBorderSize =
        1.0f;

    style.FramePadding =
        ImVec2(
            8.0f,
            6.0f
        );

    style.ItemSpacing =
        ImVec2(
            8.0f,
            8.0f
        );

    ImVec4* colors =
        style.Colors;

    colors[ImGuiCol_Text] =
        ImVec4(
            0.82f,
            0.94f,
            0.98f,
            1.0f
        );

    colors[ImGuiCol_TextDisabled] =
        ImVec4(
            0.35f,
            0.55f,
            0.62f,
            1.0f
        );

    colors[ImGuiCol_WindowBg] =
        ImVec4(
            0.008f,
            0.018f,
            0.032f,
            1.0f
        );

    colors[ImGuiCol_ChildBg] =
        ImVec4(
            0.012f,
            0.032f,
            0.052f,
            0.97f
        );

    colors[ImGuiCol_Border] =
        ImVec4(
            0.05f,
            0.38f,
            0.48f,
            0.55f
        );

    colors[ImGuiCol_FrameBg] =
        ImVec4(
            0.02f,
            0.08f,
            0.11f,
            1.0f
        );

    colors[ImGuiCol_FrameBgHovered] =
        ImVec4(
            0.03f,
            0.16f,
            0.20f,
            1.0f
        );

    colors[ImGuiCol_Button] =
        ImVec4(
            0.02f,
            0.17f,
            0.21f,
            1.0f
        );

    colors[ImGuiCol_ButtonHovered] =
        ImVec4(
            0.04f,
            0.32f,
            0.38f,
            1.0f
        );

    colors[ImGuiCol_ButtonActive] =
        ImVec4(
            0.04f,
            0.43f,
            0.50f,
            1.0f
        );

    colors[ImGuiCol_Header] =
        ImVec4(
            0.02f,
            0.18f,
            0.23f,
            1.0f
        );

    colors[ImGuiCol_HeaderHovered] =
        ImVec4(
            0.03f,
            0.32f,
            0.38f,
            1.0f
        );

    colors[ImGuiCol_CheckMark] =
        ImVec4(
            0.15f,
            0.88f,
            1.0f,
            1.0f
        );

    colors[ImGuiCol_SliderGrab] =
        ImVec4(
            0.10f,
            0.72f,
            0.86f,
            1.0f
        );

    colors[ImGuiCol_Separator] =
        ImVec4(
            0.05f,
            0.36f,
            0.44f,
            0.50f
        );
}

bool VulkanRenderer::recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    const VulkanSwapchain& swapchain
)
{
    VkCommandBufferBeginInfo
        beginInfo{};

    beginInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (
        vkBeginCommandBuffer(
            commandBuffer,
            &beginInfo
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    std::array<
        VkClearValue,
        2
    > clearValues{};

    clearValues[0].color = {{
        0.003f,
        0.010f,
        0.020f,
        1.0f
    }};

    clearValues[1].depthStencil = {
        1.0f,
        0
    };

    VkRenderPassBeginInfo
        renderPassInfo{};

    renderPassInfo.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    renderPassInfo.renderPass =
        renderPass_;

    renderPassInfo.framebuffer =
        framebuffers_[imageIndex];

    renderPassInfo.renderArea.offset =
        {
            0,
            0
        };

    renderPassInfo.renderArea.extent =
        swapchain.extent();

    renderPassInfo.clearValueCount =
        static_cast<uint32_t>(
            clearValues.size()
        );

    renderPassInfo.pClearValues =
        clearValues.data();

    vkCmdBeginRenderPass(
        commandBuffer,
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    if (
        brainInitialized_ &&
        ui_.cortexVisible()
    )
    {
        const NeuroUI::BrainViewport
            uiViewport =
                ui_.brainViewport();

        const ImGuiIO& io =
            ImGui::GetIO();

        BrainMesh::Viewport
            brainViewport{};

        brainViewport.x =
            uiViewport.x *
            io.DisplayFramebufferScale.x;

        brainViewport.y =
            uiViewport.y *
            io.DisplayFramebufferScale.y;

        brainViewport.width =
            uiViewport.width *
            io.DisplayFramebufferScale.x;

        brainViewport.height =
            uiViewport.height *
            io.DisplayFramebufferScale.y;

        if (
            brainViewport.x <
            static_cast<float>(
                swapchain.extent().width
            ) &&
            brainViewport.y <
            static_cast<float>(
                swapchain.extent().height
            )
        )
        {
            brainViewport.width =
                std::min(
                    brainViewport.width,
                    static_cast<float>(
                        swapchain.extent().width
                    ) -
                    brainViewport.x
                );

            brainViewport.height =
                std::min(
                    brainViewport.height,
                    static_cast<float>(
                        swapchain.extent().height
                    ) -
                    brainViewport.y
                );

            BrainMesh::RenderSettings
                settings{};

            settings.opacity =
                ui_.cortexOpacity();

            settings.brightness =
                ui_.activityIntensity();

            settings.activityEnabled =
                ui_.activityMapEnabled()
                    ? 1.0f
                    : 0.0f;

            settings.time =
                static_cast<float>(
                    ImGui::GetTime()
                );

            brainMesh_.draw(
                commandBuffer,
                camera_,
                brainViewport,
                settings
            );
        }
    }

    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        commandBuffer
    );

    vkCmdEndRenderPass(
        commandBuffer
    );

    return
        vkEndCommandBuffer(
            commandBuffer
        ) == VK_SUCCESS;
}

void VulkanRenderer::drawFrame(
    VulkanDevice& device,
    VulkanSwapchain& swapchain
)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ui_.render();

    const NeuroUI::BrainViewport
        brainViewport =
            ui_.brainViewport();

    ImGuiIO& io =
        ImGui::GetIO();

    if (
        ui_.consumeResetViewRequest()
    )
    {
        camera_ =
            Camera{};
    }

    camera_.update(
        io.MouseDelta.x,
        io.MouseDelta.y,
        io.MouseWheel,
        brainViewport.hovered,
        brainViewport.dragging
    );

    ImGui::Render();

    vkWaitForFences(
        device.device(),
        1,
        &inFlightFence_,
        VK_TRUE,
        UINT64_MAX
    );

    uint32_t imageIndex =
        0;

    VkResult result =
        vkAcquireNextImageKHR(
            device.device(),
            swapchain.swapchain(),
            UINT64_MAX,
            imageAvailableSemaphore_,
            VK_NULL_HANDLE,
            &imageIndex
        );

    if (
        result != VK_SUCCESS &&
        result != VK_SUBOPTIMAL_KHR
    )
    {
        std::cerr
            << "Failed to acquire swapchain image.\n";

        return;
    }

    vkResetFences(
        device.device(),
        1,
        &inFlightFence_
    );

    vkResetCommandBuffer(
        commandBuffer_,
        0
    );

    if (
        !recordCommandBuffer(
            commandBuffer_,
            imageIndex,
            swapchain
        )
    )
    {
        std::cerr
            << "Failed to record command buffer.\n";

        return;
    }

    VkSemaphore waitSemaphores[] = {
        imageAvailableSemaphore_
    };

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSemaphore signalSemaphores[] = {
        renderFinishedSemaphore_
    };

    VkSubmitInfo submitInfo{};

    submitInfo.sType =
        VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount =
        1;

    submitInfo.pWaitSemaphores =
        waitSemaphores;

    submitInfo.pWaitDstStageMask =
        waitStages;

    submitInfo.commandBufferCount =
        1;

    submitInfo.pCommandBuffers =
        &commandBuffer_;

    submitInfo.signalSemaphoreCount =
        1;

    submitInfo.pSignalSemaphores =
        signalSemaphores;

    if (
        vkQueueSubmit(
            device.graphicsQueue(),
            1,
            &submitInfo,
            inFlightFence_
        ) != VK_SUCCESS
    )
    {
        std::cerr
            << "Failed to submit Vulkan frame.\n";

        return;
    }

    VkSwapchainKHR swapchains[] = {
        swapchain.swapchain()
    };

    VkPresentInfoKHR
        presentInfo{};

    presentInfo.sType =
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount =
        1;

    presentInfo.pWaitSemaphores =
        signalSemaphores;

    presentInfo.swapchainCount =
        1;

    presentInfo.pSwapchains =
        swapchains;

    presentInfo.pImageIndices =
        &imageIndex;

    result =
        vkQueuePresentKHR(
            device.presentQueue(),
            &presentInfo
        );

    if (
        result != VK_SUCCESS &&
        result != VK_SUBOPTIMAL_KHR
    )
    {
        std::cerr
            << "Failed to present Vulkan frame.\n";
    }
}

void VulkanRenderer::cleanup(
    VkDevice device
)
{
    if (
        device ==
        VK_NULL_HANDLE
    )
    {
        return;
    }

    vkDeviceWaitIdle(
        device
    );

    if (brainInitialized_)
    {
        brainMesh_.cleanup(
            device
        );

        brainInitialized_ =
            false;
    }

    if (imguiInitialized_)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();

        imguiInitialized_ =
            false;
    }

    if (
        imguiDescriptorPool_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyDescriptorPool(
            device,
            imguiDescriptorPool_,
            nullptr
        );

        imguiDescriptorPool_ =
            VK_NULL_HANDLE;
    }

    if (
        inFlightFence_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyFence(
            device,
            inFlightFence_,
            nullptr
        );

        inFlightFence_ =
            VK_NULL_HANDLE;
    }

    if (
        renderFinishedSemaphore_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroySemaphore(
            device,
            renderFinishedSemaphore_,
            nullptr
        );

        renderFinishedSemaphore_ =
            VK_NULL_HANDLE;
    }

    if (
        imageAvailableSemaphore_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroySemaphore(
            device,
            imageAvailableSemaphore_,
            nullptr
        );

        imageAvailableSemaphore_ =
            VK_NULL_HANDLE;
    }

    for (
        VkFramebuffer framebuffer :
        framebuffers_
    )
    {
        vkDestroyFramebuffer(
            device,
            framebuffer,
            nullptr
        );
    }

    framebuffers_.clear();

    if (
        depthImageView_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyImageView(
            device,
            depthImageView_,
            nullptr
        );

        depthImageView_ =
            VK_NULL_HANDLE;
    }

    if (
        depthImage_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyImage(
            device,
            depthImage_,
            nullptr
        );

        depthImage_ =
            VK_NULL_HANDLE;
    }

    if (
        depthImageMemory_ !=
        VK_NULL_HANDLE
    )
    {
        vkFreeMemory(
            device,
            depthImageMemory_,
            nullptr
        );

        depthImageMemory_ =
            VK_NULL_HANDLE;
    }

    if (
        commandPool_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyCommandPool(
            device,
            commandPool_,
            nullptr
        );

        commandPool_ =
            VK_NULL_HANDLE;
    }

    if (
        renderPass_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyRenderPass(
            device,
            renderPass_,
            nullptr
        );

        renderPass_ =
            VK_NULL_HANDLE;
    }
}