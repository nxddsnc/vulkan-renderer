#include "Platform.h"

class Framebuffer;
class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

private:
	std::shared_ptr<Framebuffer>			m_pShadowMapFramebuffer;
};