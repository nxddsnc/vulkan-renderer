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
};

