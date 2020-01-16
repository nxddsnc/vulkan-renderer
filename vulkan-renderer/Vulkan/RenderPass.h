#include "Platform.h"

class RenderPass
{
public:
    RenderPass();
    ~RenderPass();

    vk::RenderPass Get();
    void AddAttachment(vk::AttachmentDescription attachmentDescription);
    void AddSubPass(vk::SubpassDescription subpass);
private:
    std::vector<vk::AttachmentDescription> _attachments;
    std::vector<vk::SubpassDescription> _subpasses;

    vk::Device     _device;
    vk::RenderPass _renderPass;
};
