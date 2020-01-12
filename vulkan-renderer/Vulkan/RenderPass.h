#include "Platform.h"

class RenderPass
{
public:
    RenderPass();
    ~RenderPass();

    vk::RenderPass Build();
    void AddAttachment(vk::AttachmentDescription attachmentDescription);
private:
    std::vector<vk::AttachmentDescription> _attachments;
    std::vector<vk::SubpassDescription> _subpasses;

    vk::Device     _device;
    vk::RenderPass _renderPass;
};
