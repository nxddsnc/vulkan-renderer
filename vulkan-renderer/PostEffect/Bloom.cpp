#include "Bloom.h"
#include "ResourceManager.h"
#include "Framebuffer.h"
#include "PipelineManager.h"
#include "MyImage.h"
#include "Drawable.h"
#include "MyTexture.h"

Bloom::Bloom(ResourceManager *resourceManager, PipelineManager *pipelineManager, std::shared_ptr<Framebuffer> inputFramebuffer) :
	PostEffect(resourceManager, pipelineManager, inputFramebuffer)
{
	_init();
}

Bloom::~Bloom()
{
	_deInit();
}

void Bloom::Draw(vk::CommandBuffer commandBuffer)
{

}

void Bloom::_init()
{
	m_framebuffer1 = std::make_shared<Framebuffer>("bloom-1", m_pResourceManager,
		MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT,
		m_pInputFramebuffer->m_pColorTexture->texture->m_pImage->m_width , m_pInputFramebuffer->m_pColorTexture->texture->m_pImage->m_width);

	m_framebuffer2 = std::make_shared<Framebuffer>("bloom-2", m_pResourceManager,
		MyImageFormat::MY_IMAGEFORMAT_RGBA16_FLOAT, MyImageFormat::MY_IMAGEFORMAT_D24S8_UINT,
		m_pInputFramebuffer->m_pColorTexture->texture->m_pImage->m_width, m_pInputFramebuffer->m_pColorTexture->texture->m_pImage->m_width);
}

void Bloom::_deInit()
{
	m_framebuffer1 = nullptr;
	m_framebuffer2 = nullptr;
}
