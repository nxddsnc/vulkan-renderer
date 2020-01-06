#include "renderer.h"
#include "Window.h"
#include <array>
#include <chrono>
#include <iostream>
#include <GLFW\glfw3.h>
#include "Context.h"
#include "Scene.h"
#include "ModelLoader.h"

Renderer *renderer;
Window *window;
double mouseX, mouseY;

void ResizeCallback(GLFWwindow* window, int width, int height)
{
	renderer->Resize(width, height);
}

void CloseCallback(GLFWwindow* _window)
{
	window->Close();
}

void MouseButtonCallback(GLFWwindow* _window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_1)
    {
        if (action == GLFW_PRESS)
        {
            renderer->GetCamera()->keys.left = true;
        }
        else
        {
            renderer->GetCamera()->keys.left = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_2)
    {
        if (action == GLFW_PRESS)
        {
            renderer->GetCamera()->keys.left = true;
        }
        else
        {
            renderer->GetCamera()->keys.left = false;
        }
    }
}

void MouseScrollCallback(GLFWwindow* _window, double xoffset, double yoffset)
{
    float wheelDelta = yoffset;
    renderer->GetCamera()->translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f));
}

void MouseMoveCallback(GLFWwindow* _window, double xpos, double ypos)
{
    double dx = mouseX - xpos;
    double dy = mouseY - ypos;

    mouseX = xpos;
    mouseY = ypos;
    
    Camera *camera = renderer->GetCamera();
    if (camera->keys.left) {
        camera->rotate(glm::vec3(dy * camera->rotationSpeed, -dx * camera->rotationSpeed, 0.0f));
    }
}

int main()
{
	window = new Window(WIDTH, HEIGHT, "Vulkan_Renderer");
	renderer = new Renderer(window);
    GLFWwindow *_window = window->GetGLFWWindow();
	glfwSetWindowSizeCallback(_window, ResizeCallback);
	glfwSetWindowCloseCallback(_window, CloseCallback);
    glfwSetMouseButtonCallback(_window, MouseButtonCallback);
    glfwSetScrollCallback(_window, MouseScrollCallback);
    glfwSetCursorPosCallback(_window, MouseMoveCallback);

    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    uint64_t frame_counter = 0;
    uint64_t fps = 0;

    MyScene myScene;
    ModelLoader modelLoader(&myScene);
    if (!modelLoader.load("./TestModel/DamagedHelmet/DamagedHelmet.gltf"))
    {
        return -1;
    } 
    else
    {
        renderer->addRenderNodes(myScene.GetNodes());
    }

	while (renderer->Run())
	{
        ++frame_counter;
        if (last_time + std::chrono::seconds(1) < timer.now()) {
            last_time = timer.now();
            fps = frame_counter;
            frame_counter = 0;
            std::cout << "FPS: " << fps << std::endl;
        }
		renderer->DrawFrame();

        glfwPollEvents();
	}
	delete renderer;
	return 0;
}
