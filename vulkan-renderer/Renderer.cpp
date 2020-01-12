#include "Renderer.h"
Renderer::Renderer()
{
}
Renderer::Renderer(Window *window)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Run()
{
    return false;
}

void Renderer::DrawFrame()
{
}

void Renderer::Resize(int width, int height)
{
}

Camera * Renderer::GetCamera()
{
    return nullptr;
}

void Renderer::OnSceneChanged()
{
}

void Renderer::GetExtendSize(uint32_t &width, uint32_t &height)
{

}
