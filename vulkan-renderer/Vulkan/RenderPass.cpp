#include "RenderPass.h"

RenderPass::RenderPass(vk::Device *device)
{
  _renderPass = nullptr;
  _device = device; 
}

RenderPass::~RenderPass()
{

}

void RenderPass::AddAttachment(vk::AttachmentDescription attachmentDescription)
{
  _attachments.push_back(attachmentDescription);
}

void RenderPass::Destory()
{
    _device->destroyRenderPass(_renderPass);
}

vk::RenderPass RenderPass::Get() 
{
    if (_renderPass) 
    {
      return _renderPass;
    }

    vk::SubpassDependency dependency({
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead,
        {}
    });

    vk::AttachmentReference subpassColorAttachment({
        0,
        vk::ImageLayout::eColorAttachmentOptimal
    });

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &subpassColorAttachment;

    // TODO: modified for multiple color attachments.
    if (_attachments.size() == 2)
    {
        vk::AttachmentReference subpassDepthStencilAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        subpass.pDepthStencilAttachment = &subpassDepthStencilAttachment;
    }

    vk::RenderPassCreateInfo createInfo({
        {},
        static_cast<uint32_t>(_attachments.size()),
        _attachments.data(),
        1,
        &subpass,
        1,
        &dependency
    });

    _renderPass = _device->createRenderPass(createInfo);
    return _renderPass;
}