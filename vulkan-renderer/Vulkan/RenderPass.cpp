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
  
	vk::AttachmentReference subpassDepthStencilAttachment({
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	});

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	std::array<vk::AttachmentReference, 1> subpassColorAttachments{};
	subpassColorAttachments[0].attachment = 0;
	subpassColorAttachments[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

	std::array<vk::SubpassDescription, 1> subpasses{};
	subpasses[0].pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpasses[0].inputAttachmentCount = 0;
	subpasses[0].pInputAttachments = VK_NULL_HANDLE;
	subpasses[0].colorAttachmentCount = subpassColorAttachments.size();
	subpasses[0].pColorAttachments = subpassColorAttachments.data();
	subpasses[0].pDepthStencilAttachment = &subpassDepthStencilAttachment;

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
		_attachments.size(),
		_attachments.data(),
		_subpasses.size(),
		_subpasses.data(),
		1,
		&dependency
	});

	_renderPass = _device.createRenderPass(createInfo);
  return _renderPass;
}