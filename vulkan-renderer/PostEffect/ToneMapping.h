#include "PostEffect/PostEffect.h"

class ResourceManager;
class PipelineManager;
class Framebuffer;
class ToneMapping : public PostEffect
{
public:
	ToneMapping(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height);
	~ToneMapping();

	void Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, std::shared_ptr<Framebuffer> outputFramebuffer = nullptr);
	std::shared_ptr<Framebuffer> GetFramebuffer();
private:
	std::shared_ptr<Framebuffer>		m_framebuffer;
private:
	void _init();
	void _deInit();
};