#include "PostEffect/PostEffect.h"

class ResourceManager;
class PipelineManager;
class Framebuffer;
class Pipeline;
class Bloom : public PostEffect
{
public:
	Bloom(ResourceManager *resourceManager, PipelineManager *pipelineManager, int width, int height);
	~Bloom();

	void Draw(vk::CommandBuffer commandBuffer, std::shared_ptr<Framebuffer> inputFramebuffer, std::shared_ptr<Framebuffer> outputFramebuffer = nullptr);
	std::shared_ptr<Framebuffer> GetFramebuffer();
private:
	std::shared_ptr<Framebuffer>		m_framebuffer1;
	std::shared_ptr<Framebuffer>		m_framebuffer2;
	std::shared_ptr<Pipeline>			m_pPipelineBrightness;
	std::shared_ptr<Pipeline>			m_pPipelineBlurX;
	std::shared_ptr<Pipeline>			m_pPipelineBlurY;

	float								m_brightnessThreshold;
	float								m_parameters[4]; // scale, strength, directionX, directionY
private:
	void _init();
	void _deInit();
};