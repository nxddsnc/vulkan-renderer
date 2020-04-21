#include "RenderPass.h"

RenderPass::RenderPass(vk::Device *device)
{
  _renderPass = nullptr;
  _device = device; 
  m_depthStencilAttachment = NULL;
}

RenderPass::~RenderPass()
{
	_device->destroyRenderPass(_renderPass);
}

void RenderPass::AddAttachment(vk::AttachmentDescription attachmentDescription)
{
  _attachments.push_back(attachmentDescription);
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


	vk::SubpassDescription subpass;

	for (int i = 0; i < _attachments.size(); ++i)
	{
		if (_attachments[i].format == vk::Format::eD24UnormS8Uint || 
			_attachments[i].format == vk::Format::eD16UnormS8Uint ||
			_attachments[i].format == vk::Format::eD32SfloatS8Uint ||
			_attachments[i].format == vk::Format::eD32Sfloat)
		{
			vk::AttachmentReference subpassDepthStencilAttachment(i, vk::ImageLayout::eDepthStencilAttachmentOptimal);
			subpass.pDepthStencilAttachment = &subpassDepthStencilAttachment;
			m_depthStencilAttachment = subpassDepthStencilAttachment;
		}
		else
		{
			vk::AttachmentReference subpassColorAttachment(i, vk::ImageLayout::eColorAttachmentOptimal);
			m_colorAttachments.push_back(subpassColorAttachment);
		}
	}

    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = m_colorAttachments.size();
    subpass.pColorAttachments = m_colorAttachments.data();

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