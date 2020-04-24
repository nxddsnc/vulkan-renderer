#include "VulkanRenderer.h"
#include "Window.h"
#include <array>
#include <chrono>
#include <iostream>
#include <GLFW\glfw3.h>
#include "MyScene.h"
#include "ModelLoader.h"
#include "MyCamera.h"

VulkanRenderer *renderer;
Window *window;
double mouseX, mouseY;

std::string envMaps[2] = { "./TestModel/Skybox/environment.dds", "./TestModel/Skybox/country.dds" };
int envMapIndex = 0;

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
            renderer->GetCamera()->m_keys.left = true;
        }
        else
        {
            renderer->GetCamera()->m_keys.left = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_2)
    {
        if (action == GLFW_PRESS)
        {
            renderer->GetCamera()->m_keys.left = true;
        }
        else
        {
            renderer->GetCamera()->m_keys.left = false;
        }
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (GLFW_PRESS == action)
	{
		switch (key)
		{
		case GLFW_KEY_S:
			// switch skybox.
			printf("*****************************Key s pressed. Switching skybox.******************************\n");
			envMapIndex = (envMapIndex + 1) % 2;
			renderer->LoadSkybox(envMaps[envMapIndex].c_str());
			printf("*****************************Switching skybox complete.******************************\n");
			break;
		default:
			break;
		}
	}
}

void MouseScrollCallback(GLFWwindow* _window, double xoffset, double yoffset)
{
    float wheelDelta = yoffset;
    renderer->GetCamera()->Translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * 200));
}

void MouseMoveCallback(GLFWwindow* _window, double xpos, double ypos)
{
    double dx = mouseX - xpos;
    double dy = mouseY - ypos;

    mouseX = xpos;
    mouseY = ypos;
    
    std::shared_ptr<MyCamera> camera = renderer->GetCamera();
    if (camera->m_keys.left) {
        camera->Rotate(glm::vec3(-dy * camera->m_rotationSpeed, 0.0f, -dx * camera->m_rotationSpeed));
    }
}

int main()
{
    window = new Window(WIDTH, HEIGHT, "Vulkan_Renderer");
    renderer = new VulkanRenderer(window);
	renderer->LoadSkybox(envMaps[envMapIndex].c_str());
    GLFWwindow *_window = window->GetGLFWWindow();
    glfwSetWindowSizeCallback(_window, ResizeCallback);
    glfwSetWindowCloseCallback(_window, CloseCallback);
    glfwSetMouseButtonCallback(_window, MouseButtonCallback);
	glfwSetKeyCallback(_window, KeyCallback);
    glfwSetScrollCallback(_window, MouseScrollCallback);
    glfwSetCursorPosCallback(_window, MouseMoveCallback);

    auto timer = std::chrono::steady_clock();
    auto lastTime = timer.now();
    auto lastFrameTime = timer.now();
    uint64_t frameCounter = 0;
    uint64_t fps = 0;

	std::shared_ptr<MyScene> myScene = std::make_shared<MyScene>();
    ModelLoader modelLoader(myScene);
    //if (!modelLoader.load("./TestModel/damagedHelmet/damagedHelmet.gltf"))
    //if (!modelLoader.load("./TestModel/cube/Cube.gltf"))
    //if (!modelLoader.load("./TestModel/sphere1.obj"))
	//if (!modelLoader.load("./TestModel/house.fbx"))
	if (!modelLoader.load("./TestModel/BrainStem/BrainStem.gltf"))
	//if (!modelLoader.load("./TestModel/walking/scene.gltf"))
    {
        std::cout << "can't read gltf file!" << std::endl;
        return -1;
    } 
    else
    {
        renderer->AddScene(myScene);
    }

    while (renderer->Run())
    {
        if (lastFrameTime + std::chrono::milliseconds(16) < timer.now())
        {
            lastFrameTime = timer.now();
        }
        else
        {
            continue;
        }

        ++frameCounter;
        if (lastTime + std::chrono::seconds(1) < timer.now()) {
            lastTime = timer.now();
            fps = frameCounter;
            frameCounter = 0;
            std::cout << "FPS: " << fps << std::endl;
        }
        renderer->DrawFrame();

        glfwPollEvents();
    }
    delete renderer;
    return 0;
}
