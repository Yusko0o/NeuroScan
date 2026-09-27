#pragma once
#include "renderer/BrainMesh.hpp"
#include <QObject>
#include <QSGSimpleTextureNode>
#include <QPointer>
#include <QSize>
class BrainViewport;
class QQuickWindow;

// Scene-graph owned: construction, synchronization, rendering and destruction
// all take place on Qt's render thread. Qt owns the device and command buffer.
class BrainTextureNode : public QObject, public QSGSimpleTextureNode {
public:
    explicit BrainTextureNode(BrainViewport* item);
    ~BrainTextureNode() override;
    void sync(BrainViewport* item);
private:
    struct Image {
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };
    void initialize();
    void resize(QSize size);
    void render();
    void createImage(Image&, VkFormat, VkImageUsageFlags, VkImageAspectFlags);
    void freeImage(Image&);
    void freeTarget();
    void publish(QString message);
    QQuickWindow* window_;
    QPointer<BrainViewport> item_;
    VkDevice device_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_ = VK_NULL_HANDLE;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
    VkFormat depthFormat_ = VK_FORMAT_UNDEFINED;
    Image color_, depth_;
    QSize size_;
    QSGTexture* wrapper_ = nullptr;
    QString deviceName_;
    BrainMesh mesh_;
    Camera camera_;
    BrainMesh::RenderSettings settings_;
    bool ready_ = false, failed_ = false, visible_ = true;
};
