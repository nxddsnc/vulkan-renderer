#include "Platform.h"

class RenderPass
{
public:
    RenderPass(vk::Device *device);
    ~RenderPass();

    vk::RenderPass Get();
    void AddAttachment(vk::AttachmentDescription attachmentDescription);
    void Destory();
private:
    std::vector<vk::AttachmentDescription> _attachments;
    std::vector<vk::SubpassDescription> _subpasses;

    vk::Device     *_device;
    vk::RenderPass _renderPass;
};
