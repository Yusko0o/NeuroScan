#include "BrainMesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

bool BrainMesh::initialize(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkRenderPass renderPass,
    const std::string& modelPath,
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath
)
{
    if (!loadModel(modelPath))
    {
        return false;
    }

    if (
        !createVertexBuffer(
            physicalDevice,
            device
        )
    )
    {
        return false;
    }

    if (
        !createIndexBuffer(
            physicalDevice,
            device
        )
    )
    {
        return false;
    }

    if (
        !createGraphicsPipeline(
            device,
            renderPass,
            vertexShaderPath,
            fragmentShaderPath
        )
    )
    {
        return false;
    }

    std::cout
        << "Brain mesh ready: "
        << vertices_.size()
        << " vertices, "
        << indices_.size() / 3
        << " triangles.\n";

    return true;
}

bool BrainMesh::loadModel(
    const std::string& path
)
{
    vertices_.clear();
    indices_.clear();

    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_PreTransformVertices
        );

    if (
        scene == nullptr ||
        scene->mRootNode == nullptr
    )
    {
        std::cerr
            << "Failed to load brain model: "
            << importer.GetErrorString()
            << "\n";

        return false;
    }

    glm::vec3 minimum(
        std::numeric_limits<float>::max()
    );

    glm::vec3 maximum(
        std::numeric_limits<float>::lowest()
    );

    uint32_t vertexOffset = 0;

    for (
        unsigned int meshIndex = 0;
        meshIndex < scene->mNumMeshes;
        ++meshIndex
    )
    {
        const aiMesh* mesh =
            scene->mMeshes[meshIndex];

        for (
            unsigned int i = 0;
            i < mesh->mNumVertices;
            ++i
        )
        {
            Vertex vertex{};

            vertex.position = {
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            };

            if (mesh->HasNormals())
            {
                vertex.normal = {
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                };
            }
            else
            {
                vertex.normal =
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f
                    );
            }

            minimum =
                glm::min(
                    minimum,
                    vertex.position
                );

            maximum =
                glm::max(
                    maximum,
                    vertex.position
                );

            vertices_.push_back(
                vertex
            );
        }

        for (
            unsigned int faceIndex = 0;
            faceIndex < mesh->mNumFaces;
            ++faceIndex
        )
        {
            const aiFace& face =
                mesh->mFaces[faceIndex];

            if (face.mNumIndices != 3)
            {
                continue;
            }

            indices_.push_back(
                vertexOffset +
                face.mIndices[0]
            );

            indices_.push_back(
                vertexOffset +
                face.mIndices[1]
            );

            indices_.push_back(
                vertexOffset +
                face.mIndices[2]
            );
        }

        vertexOffset +=
            mesh->mNumVertices;
    }

    if (
        vertices_.empty() ||
        indices_.empty()
    )
    {
        std::cerr
            << "Brain model contains no renderable geometry.\n";

        return false;
    }

    const glm::vec3 center =
        (minimum + maximum) *
        0.5f;

    const glm::vec3 size =
        maximum - minimum;

    const float largestDimension =
        std::max(
            size.x,
            std::max(
                size.y,
                size.z
            )
        );

    if (largestDimension <= 0.0f)
    {
        return false;
    }

    const float scale =
        2.0f /
        largestDimension;

    for (
        Vertex& vertex :
        vertices_
    )
    {
        vertex.position =
            (
                vertex.position -
                center
            ) *
            scale;

        vertex.normal =
            glm::normalize(
                vertex.normal
            );
    }

    return true;
}

bool BrainMesh::createBuffer(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& memory
)
{
    VkBufferCreateInfo
        bufferInfo{};

    bufferInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferInfo.size =
        size;

    bufferInfo.usage =
        usage;

    bufferInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;

    if (
        vkCreateBuffer(
            device,
            &bufferInfo,
            nullptr,
            &buffer
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    VkMemoryRequirements
        requirements{};

    vkGetBufferMemoryRequirements(
        device,
        buffer,
        &requirements
    );

    VkMemoryAllocateInfo
        allocationInfo{};

    allocationInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocationInfo.allocationSize =
        requirements.size;

    allocationInfo.memoryTypeIndex =
        findMemoryType(
            physicalDevice,
            requirements.memoryTypeBits,
            properties
        );

    if (
        vkAllocateMemory(
            device,
            &allocationInfo,
            nullptr,
            &memory
        ) != VK_SUCCESS
    )
    {
        vkDestroyBuffer(
            device,
            buffer,
            nullptr
        );

        buffer =
            VK_NULL_HANDLE;

        return false;
    }

    if (
        vkBindBufferMemory(
            device,
            buffer,
            memory,
            0
        ) != VK_SUCCESS
    )
    {
        vkDestroyBuffer(
            device,
            buffer,
            nullptr
        );

        vkFreeMemory(
            device,
            memory,
            nullptr
        );

        buffer =
            VK_NULL_HANDLE;

        memory =
            VK_NULL_HANDLE;

        return false;
    }

    return true;
}

uint32_t BrainMesh::findMemoryType(
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
        "No suitable Vulkan memory type found."
    );
}

bool BrainMesh::createVertexBuffer(
    VkPhysicalDevice physicalDevice,
    VkDevice device
)
{
    const VkDeviceSize size =
        sizeof(Vertex) *
        vertices_.size();

    if (
        !createBuffer(
            physicalDevice,
            device,
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer_,
            vertexMemory_
        )
    )
    {
        return false;
    }

    void* mapped =
        nullptr;

    if (
        vkMapMemory(
            device,
            vertexMemory_,
            0,
            size,
            0,
            &mapped
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    std::memcpy(
        mapped,
        vertices_.data(),
        static_cast<size_t>(
            size
        )
    );

    vkUnmapMemory(
        device,
        vertexMemory_
    );

    return true;
}

bool BrainMesh::createIndexBuffer(
    VkPhysicalDevice physicalDevice,
    VkDevice device
)
{
    const VkDeviceSize size =
        sizeof(uint32_t) *
        indices_.size();

    if (
        !createBuffer(
            physicalDevice,
            device,
            size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexBuffer_,
            indexMemory_
        )
    )
    {
        return false;
    }

    void* mapped =
        nullptr;

    if (
        vkMapMemory(
            device,
            indexMemory_,
            0,
            size,
            0,
            &mapped
        ) != VK_SUCCESS
    )
    {
        return false;
    }

    std::memcpy(
        mapped,
        indices_.data(),
        static_cast<size_t>(
            size
        )
    );

    vkUnmapMemory(
        device,
        indexMemory_
    );

    return true;
}

std::vector<char> BrainMesh::readFile(
    const std::string& path
)
{
    std::ifstream file(
        path,
        std::ios::ate |
        std::ios::binary
    );

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Failed to open shader: " +
            path
        );
    }

    const std::streamsize size =
        file.tellg();

    if (size <= 0)
    {
        throw std::runtime_error(
            "Shader file is empty: " +
            path
        );
    }

    std::vector<char> buffer(
        static_cast<size_t>(
            size
        )
    );

    file.seekg(
        0
    );

    file.read(
        buffer.data(),
        size
    );

    return buffer;
}

VkShaderModule BrainMesh::createShaderModule(
    VkDevice device,
    const std::vector<char>& code
)
{
    VkShaderModuleCreateInfo
        info{};

    info.sType =
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    info.codeSize =
        code.size();

    info.pCode =
        reinterpret_cast<
            const uint32_t*
        >(
            code.data()
        );

    VkShaderModule module =
        VK_NULL_HANDLE;

    if (
        vkCreateShaderModule(
            device,
            &info,
            nullptr,
            &module
        ) != VK_SUCCESS
    )
    {
        throw std::runtime_error(
            "Failed to create shader module."
        );
    }

    return module;
}

bool BrainMesh::createGraphicsPipeline(
    VkDevice device,
    VkRenderPass renderPass,
    const std::string& vertexShaderPath,
    const std::string& fragmentShaderPath
)
{
    const auto vertexCode =
        readFile(
            vertexShaderPath
        );

    const auto fragmentCode =
        readFile(
            fragmentShaderPath
        );

    const VkShaderModule vertexModule =
        createShaderModule(
            device,
            vertexCode
        );

    const VkShaderModule fragmentModule =
        createShaderModule(
            device,
            fragmentCode
        );

    VkPipelineShaderStageCreateInfo
        vertexStage{};

    vertexStage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    vertexStage.stage =
        VK_SHADER_STAGE_VERTEX_BIT;

    vertexStage.module =
        vertexModule;

    vertexStage.pName =
        "main";

    VkPipelineShaderStageCreateInfo
        fragmentStage{};

    fragmentStage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    fragmentStage.stage =
        VK_SHADER_STAGE_FRAGMENT_BIT;

    fragmentStage.module =
        fragmentModule;

    fragmentStage.pName =
        "main";

    const std::array<
        VkPipelineShaderStageCreateInfo,
        2
    > stages = {
        vertexStage,
        fragmentStage
    };

    VkVertexInputBindingDescription
        binding{};

    binding.binding =
        0;

    binding.stride =
        sizeof(Vertex);

    binding.inputRate =
        VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<
        VkVertexInputAttributeDescription,
        2
    > attributes{};

    attributes[0].binding =
        0;

    attributes[0].location =
        0;

    attributes[0].format =
        VK_FORMAT_R32G32B32_SFLOAT;

    attributes[0].offset =
        offsetof(
            Vertex,
            position
        );

    attributes[1].binding =
        0;

    attributes[1].location =
        1;

    attributes[1].format =
        VK_FORMAT_R32G32B32_SFLOAT;

    attributes[1].offset =
        offsetof(
            Vertex,
            normal
        );

    VkPipelineVertexInputStateCreateInfo
        vertexInput{};

    vertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInput.vertexBindingDescriptionCount =
        1;

    vertexInput.pVertexBindingDescriptions =
        &binding;

    vertexInput.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(
            attributes.size()
        );

    vertexInput.pVertexAttributeDescriptions =
        attributes.data();

    VkPipelineInputAssemblyStateCreateInfo
        inputAssembly{};

    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    inputAssembly.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    inputAssembly.primitiveRestartEnable =
        VK_FALSE;

    VkPipelineViewportStateCreateInfo
        viewportState{};

    viewportState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    viewportState.viewportCount =
        1;

    viewportState.scissorCount =
        1;

    VkPipelineRasterizationStateCreateInfo
        rasterizer{};

    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    rasterizer.depthClampEnable =
        VK_FALSE;

    rasterizer.rasterizerDiscardEnable =
        VK_FALSE;

    rasterizer.polygonMode =
        VK_POLYGON_MODE_FILL;

    rasterizer.lineWidth =
        1.0f;

    rasterizer.cullMode =
        VK_CULL_MODE_NONE;

    rasterizer.frontFace =
        VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo
        multisampling{};

    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    multisampling.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;

    multisampling.sampleShadingEnable =
        VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo
        depthStencil{};

    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    depthStencil.depthTestEnable =
        VK_TRUE;

    depthStencil.depthWriteEnable =
        VK_TRUE;

    depthStencil.depthCompareOp =
        VK_COMPARE_OP_LESS;

    depthStencil.depthBoundsTestEnable =
        VK_FALSE;

    depthStencil.stencilTestEnable =
        VK_FALSE;

    VkPipelineColorBlendAttachmentState
        blendAttachment{};

    blendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    blendAttachment.blendEnable =
        VK_TRUE;

    blendAttachment.srcColorBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;

    blendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    blendAttachment.colorBlendOp =
        VK_BLEND_OP_ADD;

    blendAttachment.srcAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE;

    blendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    blendAttachment.alphaBlendOp =
        VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo
        blending{};

    blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    blending.logicOpEnable =
        VK_FALSE;

    blending.attachmentCount =
        1;

    blending.pAttachments =
        &blendAttachment;

    const std::array<
        VkDynamicState,
        2
    > dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo
        dynamicState{};

    dynamicState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(
            dynamicStates.size()
        );

    dynamicState.pDynamicStates =
        dynamicStates.data();

    VkPushConstantRange
        pushConstant{};

    pushConstant.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT |
        VK_SHADER_STAGE_FRAGMENT_BIT;

    pushConstant.offset =
        0;

    pushConstant.size =
        sizeof(PushConstants);

    VkPipelineLayoutCreateInfo
        layoutInfo{};

    layoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    layoutInfo.pushConstantRangeCount =
        1;

    layoutInfo.pPushConstantRanges =
        &pushConstant;

    if (
        vkCreatePipelineLayout(
            device,
            &layoutInfo,
            nullptr,
            &pipelineLayout_
        ) != VK_SUCCESS
    )
    {
        vkDestroyShaderModule(
            device,
            fragmentModule,
            nullptr
        );

        vkDestroyShaderModule(
            device,
            vertexModule,
            nullptr
        );

        return false;
    }

    VkGraphicsPipelineCreateInfo
        pipelineInfo{};

    pipelineInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    pipelineInfo.stageCount =
        static_cast<uint32_t>(
            stages.size()
        );

    pipelineInfo.pStages =
        stages.data();

    pipelineInfo.pVertexInputState =
        &vertexInput;

    pipelineInfo.pInputAssemblyState =
        &inputAssembly;

    pipelineInfo.pViewportState =
        &viewportState;

    pipelineInfo.pRasterizationState =
        &rasterizer;

    pipelineInfo.pMultisampleState =
        &multisampling;

    pipelineInfo.pDepthStencilState =
        &depthStencil;

    pipelineInfo.pColorBlendState =
        &blending;

    pipelineInfo.pDynamicState =
        &dynamicState;

    pipelineInfo.layout =
        pipelineLayout_;

    pipelineInfo.renderPass =
        renderPass;

    pipelineInfo.subpass =
        0;

    const VkResult result =
        vkCreateGraphicsPipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &pipeline_
        );

    vkDestroyShaderModule(
        device,
        fragmentModule,
        nullptr
    );

    vkDestroyShaderModule(
        device,
        vertexModule,
        nullptr
    );

    if (result != VK_SUCCESS)
    {
        std::cerr
            << "Failed to create brain graphics pipeline.\n";

        return false;
    }

    return true;
}

void BrainMesh::draw(
    VkCommandBuffer commandBuffer,
    const Camera& camera,
    const Viewport& viewport,
    const RenderSettings& settings
)
{
    if (
        pipeline_ == VK_NULL_HANDLE ||
        viewport.width <= 1.0f ||
        viewport.height <= 1.0f
    )
    {
        return;
    }

    VkViewport vulkanViewport{};

    vulkanViewport.x =
        viewport.x;

    vulkanViewport.y =
        viewport.y;

    vulkanViewport.width =
        viewport.width;

    vulkanViewport.height =
        viewport.height;

    vulkanViewport.minDepth =
        0.0f;

    vulkanViewport.maxDepth =
        1.0f;

    VkRect2D scissor{};

    scissor.offset.x =
        static_cast<int32_t>(
            std::max(
                0.0f,
                viewport.x
            )
        );

    scissor.offset.y =
        static_cast<int32_t>(
            std::max(
                0.0f,
                viewport.y
            )
        );

    scissor.extent.width =
        static_cast<uint32_t>(
            std::max(
                1.0f,
                viewport.width
            )
        );

    scissor.extent.height =
        static_cast<uint32_t>(
            std::max(
                1.0f,
                viewport.height
            )
        );

    vkCmdSetViewport(
        commandBuffer,
        0,
        1,
        &vulkanViewport
    );

    vkCmdSetScissor(
        commandBuffer,
        0,
        1,
        &scissor
    );

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_
    );

    VkBuffer vertexBuffers[] = {
        vertexBuffer_
    };

    VkDeviceSize offsets[] = {
        0
    };

    vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        vertexBuffers,
        offsets
    );

    vkCmdBindIndexBuffer(
        commandBuffer,
        indexBuffer_,
        0,
        VK_INDEX_TYPE_UINT32
    );

    const float aspect =
        viewport.width /
        viewport.height;

    const glm::mat4 model =
        glm::mat4(
            1.0f
        );

    const glm::mat4 view =
        camera.viewMatrix();

    const glm::mat4 projection =
        camera.projectionMatrix(
            aspect
        );

    PushConstants constants{};

    constants.mvp =
        projection *
        view *
        model;

    constants.renderSettings =
        glm::vec4(
            std::clamp(
                settings.opacity,
                0.02f,
                1.0f
            ),
            std::clamp(
                settings.brightness,
                0.0f,
                1.0f
            ),
            settings.activityEnabled,
            settings.time
        );

    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout_,
        VK_SHADER_STAGE_VERTEX_BIT |
        VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(PushConstants),
        &constants
    );

    vkCmdDrawIndexed(
        commandBuffer,
        static_cast<uint32_t>(
            indices_.size()
        ),
        1,
        0,
        0,
        0
    );
}

void BrainMesh::cleanup(
    VkDevice device
)
{
    if (
        pipeline_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyPipeline(
            device,
            pipeline_,
            nullptr
        );

        pipeline_ =
            VK_NULL_HANDLE;
    }

    if (
        pipelineLayout_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyPipelineLayout(
            device,
            pipelineLayout_,
            nullptr
        );

        pipelineLayout_ =
            VK_NULL_HANDLE;
    }

    if (
        indexBuffer_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyBuffer(
            device,
            indexBuffer_,
            nullptr
        );

        indexBuffer_ =
            VK_NULL_HANDLE;
    }

    if (
        indexMemory_ !=
        VK_NULL_HANDLE
    )
    {
        vkFreeMemory(
            device,
            indexMemory_,
            nullptr
        );

        indexMemory_ =
            VK_NULL_HANDLE;
    }

    if (
        vertexBuffer_ !=
        VK_NULL_HANDLE
    )
    {
        vkDestroyBuffer(
            device,
            vertexBuffer_,
            nullptr
        );

        vertexBuffer_ =
            VK_NULL_HANDLE;
    }

    if (
        vertexMemory_ !=
        VK_NULL_HANDLE
    )
    {
        vkFreeMemory(
            device,
            vertexMemory_,
            nullptr
        );

        vertexMemory_ =
            VK_NULL_HANDLE;
    }

    vertices_.clear();
    indices_.clear();
}