#include "BrainTextureNode.hpp"
#include "BrainViewport.hpp"
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSGTexture>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QImage>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace {
void check(VkResult result, const char* operation) {
    if (result != VK_SUCCESS)
        throw std::runtime_error(std::string(operation) + " (VkResult " + std::to_string(result) + ")");
}
template<class T> T resource(QQuickWindow* w, QSGRendererInterface::Resource kind) {
    auto* p = static_cast<T*>(w->rendererInterface()->getResource(w, kind));
    if (!p) throw std::runtime_error("Qt Vulkan resource unavailable");
    return *p;
}
}
BrainTextureNode::BrainTextureNode(BrainViewport* item) : window_(item->window()), item_(item) {
    setFiltering(QSGTexture::Linear);
    setOwnsTexture(false);
    connect(window_, &QQuickWindow::beforeRendering, this, &BrainTextureNode::render, Qt::DirectConnection);
}
BrainTextureNode::~BrainTextureNode() {
    if (device_) vkDeviceWaitIdle(device_);
    freeTarget();
    mesh_.cleanup(device_);
    if (renderPass_) vkDestroyRenderPass(device_, renderPass_, nullptr);
}
void BrainTextureNode::publish(QString message) {
    // Never mutate the GUI-thread QObject from a render callback.
    const auto target = item_;
    const auto gpu = deviceName_;
    const auto count = int(mesh_.triangleCount());
    if (target) QMetaObject::invokeMethod(target, [target, message, gpu, count] {
        if (target) target->reportStatus(message, gpu, count);
    }, Qt::QueuedConnection);
}
void BrainTextureNode::initialize() {
    if (window_->rendererInterface()->graphicsApi() != QSGRendererInterface::Vulkan)
        throw std::runtime_error("NeuroScan requires the Qt Quick Vulkan backend");
    device_ = resource<VkDevice>(window_, QSGRendererInterface::DeviceResource);
    physical_ = resource<VkPhysicalDevice>(window_, QSGRendererInterface::PhysicalDeviceResource);
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physical_, &props);
    deviceName_ = QString::fromUtf8(props.deviceName);
    for (auto format : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM}) {
        VkFormatProperties p{}; vkGetPhysicalDeviceFormatProperties(physical_, format, &p);
        if (p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) { depthFormat_ = format; break; }
    }
    if (depthFormat_ == VK_FORMAT_UNDEFINED) throw std::runtime_error("No supported depth format");
    std::array<VkAttachmentDescription, 2> attachments{};
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachments[1] = attachments[0];
    attachments[1].format = depthFormat_;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    // The same image is reused on Qt's graphics queue. Wait for the preceding
    // frame's sampling/depth writes before overwriting; expose writes to QML.
    std::array<VkSubpassDependency, 2> deps{};
    deps[0].srcSubpass = VK_SUBPASS_EXTERNAL; deps[0].dstSubpass = 0;
    deps[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
        | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    deps[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    deps[1].srcSubpass = 0; deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = uint32_t(attachments.size()); info.pAttachments = attachments.data();
    info.subpassCount = 1; info.pSubpasses = &subpass;
    info.dependencyCount = uint32_t(deps.size()); info.pDependencies = deps.data();
    check(vkCreateRenderPass(device_, &info, nullptr, &renderPass_), "create render pass");
    const QDir base(QCoreApplication::applicationDirPath());
    if (!mesh_.initialize(physical_, device_, renderPass_, base.filePath("assets/models/brain.glb").toStdString(),
        base.filePath("shaders/brain.vert.spv").toStdString(), base.filePath("shaders/brain.frag.spv").toStdString()))
        throw std::runtime_error("Brain mesh initialization failed; check model, shaders and console log");
    ready_ = true;
    qInfo().noquote() << "NeuroScan Vulkan:" << deviceName_ << "triangles:" << mesh_.triangleCount();
    publish(QStringLiteral("Vulkan actif · modèle GLB chargé"));
}
void BrainTextureNode::createImage(Image& image, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect) {
    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType = VK_IMAGE_TYPE_2D; info.format = format;
    info.extent = {uint32_t(size_.width()), uint32_t(size_.height()), 1};
    info.mipLevels = 1; info.arrayLayers = 1; info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL; info.usage = usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    check(vkCreateImage(device_, &info, nullptr, &image.image), "create viewport image");
    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(device_, image.image, &requirements);
    VkPhysicalDeviceMemoryProperties memory{};
    vkGetPhysicalDeviceMemoryProperties(physical_, &memory);
    uint32_t type = UINT32_MAX;
    for (uint32_t i = 0; i < memory.memoryTypeCount; ++i)
        if ((requirements.memoryTypeBits & (1u << i)) && (memory.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) { type = i; break; }
    if (type == UINT32_MAX) throw std::runtime_error("No device-local image memory");
    VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocation.allocationSize = requirements.size; allocation.memoryTypeIndex = type;
    check(vkAllocateMemory(device_, &allocation, nullptr, &image.memory), "allocate viewport memory");
    check(vkBindImageMemory(device_, image.image, image.memory, 0), "bind viewport image");
    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.image = image.image; view.viewType = VK_IMAGE_VIEW_TYPE_2D; view.format = format;
    view.subresourceRange = {aspect, 0, 1, 0, 1};
    check(vkCreateImageView(device_, &view, nullptr, &image.view), "create viewport view");
}
void BrainTextureNode::freeImage(Image& image) {
    if (image.view) vkDestroyImageView(device_, image.view, nullptr);
    if (image.image) vkDestroyImage(device_, image.image, nullptr);
    if (image.memory) vkFreeMemory(device_, image.memory, nullptr);
    image = {};
}
void BrainTextureNode::freeTarget() {
    delete wrapper_;
    wrapper_ = nullptr;
    if (framebuffer_) vkDestroyFramebuffer(device_, framebuffer_, nullptr);
    framebuffer_ = VK_NULL_HANDLE;
    freeImage(color_); freeImage(depth_);
}
void BrainTextureNode::resize(QSize size) {
    // Rare resize/destruction only: never stall the device in the frame loop.
    check(vkDeviceWaitIdle(device_), "wait before viewport resize");
    freeTarget(); size_ = size;
    createImage(color_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    createImage(depth_, depthFormat_, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    const VkImageView views[]{color_.view, depth_.view};
    VkFramebufferCreateInfo info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    info.renderPass = renderPass_; info.attachmentCount = 2; info.pAttachments = views;
    info.width = uint32_t(size.width()); info.height = uint32_t(size.height()); info.layers = 1;
    check(vkCreateFramebuffer(device_, &info, nullptr, &framebuffer_), "create viewport framebuffer");
    auto* wrapper = QNativeInterface::QSGVulkanTexture::fromNative(color_.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, window_, size_);
    if (!wrapper) throw std::runtime_error("Qt could not import the Vulkan texture");
    wrapper_ = wrapper;
    setTexture(wrapper_); markDirty(DirtyMaterial);
}
void BrainTextureNode::sync(BrainViewport* item) {
    if (failed_) return;
    try {
        if (!ready_) initialize();
        const auto dpr = window_->effectiveDevicePixelRatio();
        VkPhysicalDeviceProperties props{}; vkGetPhysicalDeviceProperties(physical_, &props);
        const int limit = int(std::min(props.limits.maxImageDimension2D, 4096u));
        const double physicalWidth = item->width() * dpr, physicalHeight = item->height() * dpr;
        const double scale = std::min(1.0, limit / std::max(physicalWidth, physicalHeight));
        const QSize desired(std::max(1, qRound(physicalWidth * scale)),
                            std::max(1, qRound(physicalHeight * scale)));
        if (desired != size_ || !texture()) resize(desired);
        // updatePaintNode runs with the GUI thread blocked: copy all mutable state.
        camera_ = item->camera(); settings_ = item->settings(); visible_ = item->surfaceVisible();
    } catch (const std::exception& e) {
        failed_ = true; qCritical() << e.what(); publish(QStringLiteral("Erreur : ") + QString::fromUtf8(e.what()));
        if (!wrapper_) {
            QImage background(1, 1, QImage::Format_RGB32); background.fill(QColor("#08111d"));
            wrapper_ = window_->createTextureFromImage(background); setTexture(wrapper_);
        }
    }
}
void BrainTextureNode::render() {
    if (!ready_ || failed_ || !framebuffer_) return;
    window_->beginExternalCommands();
    try {
        const auto cmd = resource<VkCommandBuffer>(window_, QSGRendererInterface::CommandListResource);
        std::array<VkClearValue, 2> clear{};
        clear[0].color = {{0.020f, 0.035f, 0.058f, 1.0f}};
        clear[1].depthStencil = {1.0f, 0};
        VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        info.renderPass = renderPass_; info.framebuffer = framebuffer_;
        info.renderArea.extent = {uint32_t(size_.width()), uint32_t(size_.height())};
        info.clearValueCount = uint32_t(clear.size()); info.pClearValues = clear.data();
        vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
        if (visible_ && settings_.opacity > 0)
            mesh_.draw(cmd, camera_, {0, 0, float(size_.width()), float(size_.height())}, settings_);
        vkCmdEndRenderPass(cmd);
    } catch (const std::exception& e) {
        failed_ = true; qCritical() << e.what(); publish(QString::fromUtf8(e.what()));
    }
    window_->endExternalCommands();
}
