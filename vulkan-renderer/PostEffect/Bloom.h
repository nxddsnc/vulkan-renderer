#include "PostEffect/PostEffect.h"

class ResourceManager;
class PipelineManager;
class Framebuffer;
class Bloom : public PostEffect
{
public:
	Bloom(ResourceManager *resourceManager, PipelineManager *pipelineManager, std::shared_ptr<Framebuffer> inputFramebuffer);
	~Bloom();

	void Draw(vk::CommandBuffer commandBuffer);
private:
	std::shared_ptr<Framebuffer>		m_framebuffer1;
	std::shared_ptr<Framebuffer>		m_framebuffer2;
private:
	void _init();
	void _deInit();
};