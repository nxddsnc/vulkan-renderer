#include "Platform.h"
class ResourceManager;
class PipelineManager;
class Framebuffer;
class PostEffect
{
public:
	PostEffect(ResourceManager *resourecManager, PipelineManager *pipelineManager, std::shared_ptr<Framebuffer> inputFramebuffer);
	virtual ~PostEffect() = 0;

	virtual void Draw(vk::CommandBuffer commandBuffer);
protected:
	ResourceManager					*m_pResourceManager;
	PipelineManager					*m_pPipelineManager;

	std::shared_ptr<Framebuffer>     m_pInputFramebuffer;



};