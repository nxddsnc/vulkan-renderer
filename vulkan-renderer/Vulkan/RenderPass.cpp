#include "RenderPass.h"

RenderPass::RenderPass()
{
  _renderPass = nullptr;
}

RenderPass::~RenderPass()
{
}

void RenderPass::AddAttachment(vk::AttachmentDescription attachmentDescription)
{
  _attachments.push_back(attachmentDescription);
}

void RenderPass::AddSubPass(vk::SubpassDescription subpass) 
{
  _subpasses.push_back(subpass);
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

	vk::RenderPassCreateInfo createInfo({
		{},
		static_cast<uint32_t>(_attachments.size()),
		_attachments.data(),
        static_cast<uint32_t>(_subpasses.size()),
		_subpasses.data(),
		1,
		&dependency
	});

	_renderPass = _device.createRenderPass(createInfo);
	return _renderPass;
}