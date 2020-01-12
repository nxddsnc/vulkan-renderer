#pragma once
class Window;
class Camera;
class Renderer
{
public:
    Renderer();
    Renderer(Window *window);

    ~Renderer();

    virtual bool Run();
    virtual void DrawFrame();
    virtual void Resize(int width, int height);
    virtual Camera * GetCamera();
    virtual void OnSceneChanged();
    virtual void GetExtendSize(uint32_t &width, uint32_t &height);
};

