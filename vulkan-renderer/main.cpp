#include "VulkanRenderer.h"
#include "Window.h"
#include <array>
#include <chrono>
#include <iostream>
#include <GLFW\glfw3.h>
#include "MyScene.h"
#include "ModelLoader.h"
#include "MyCamera.h"
#include "Context.h"
#include "ResourceManager.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

ImGui_ImplVulkanH_Window g_MainWindowData;


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
            renderer->GetCamera()->m_keys.right = true;
        }
        else
        {
            renderer->GetCamera()->m_keys.right = false;
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
    auto camera = renderer->GetCamera();
    camera->MoveForward(yoffset);
}

void MouseMoveCallback(GLFWwindow* _window, double xpos, double ypos)
{
    double dx = mouseX - xpos;
    double dy = mouseY - ypos;

    mouseX = xpos;
    mouseY = ypos;
    
    std::shared_ptr<MyCamera> camera = renderer->GetCamera();
    if (camera->m_keys.left) 
    {
        camera->Rotate(-dx, -dy);
    }
    else if (camera->m_keys.right)
    {
        camera->Translate(dx, dy);
    }
}

int main()
{
    window = new Window(WIDTH, HEIGHT, "Vulkan_Renderer");

    renderer = new VulkanRenderer(window);
	renderer->LoadSkybox(envMaps[envMapIndex].c_str());
    GLFWwindow *_window = window->GetGLFWWindow();
   
    // https://frguthmann.github.io/posts/vulkan_imgui/
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window->GetGLFWWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = renderer->GetVulkanInstance();
    init_info.PhysicalDevice = renderer->GetPhysicalDevice();
    init_info.Device = renderer->GetVulkanDevice();
    init_info.QueueFamily = renderer->GetGraphicFamilyIndex();
    init_info.Queue = renderer->GetVulkanDeviceQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = renderer->GetDescriptorPool();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = renderer->GetSwapchainImageCount();
    init_info.ImageCount = renderer->GetSwapchainImageCount();
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, renderer->GetVulkanRenderPass());

    VulkanContext* context = renderer->GetVulkanContext();
    g_MainWindowData.Width = window->GetWindowExtent().width;
    g_MainWindowData.Height = window->GetWindowExtent().height;
    g_MainWindowData.Swapchain = renderer->GetSwapchain();
    g_MainWindowData.Surface = context->GetSurface();
    g_MainWindowData.SurfaceFormat = context->GetSurfaceFormat();
    g_MainWindowData.PresentMode = (VkPresentModeKHR )renderer->GetPresentMode();
    g_MainWindowData.RenderPass = renderer->GetVulkanRenderPass();
    g_MainWindowData.Pipeline = nullptr;
    g_MainWindowData.ClearEnable = true;
    g_MainWindowData.ImageCount = init_info.ImageCount;

    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
    // Upload Fonts
    {
        vk::CommandBuffer command_buffer = renderer->GetResourceManager()->BeginSingleTimeCommand();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        renderer->GetResourceManager()->EndSingleTimeCommand(command_buffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

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
	if (!modelLoader.load("./TestModel/house.fbx"))
	//if (!modelLoader.load("./TestModel/BrainStem/BrainStem.gltf"))
	//if (!modelLoader.load("./TestModel/walking/scene.gltf"))
    //if (!modelLoader.load("./TestModel/crytek-sponza-huge-vray.fbx"))
    {
        std::cout << "can't read file!" << std::endl;
        return -1;
    } 
    else
    {
        renderer->AddScene(myScene);
    }

    while (renderer->Run())
    {
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            /*memcpy(&wd->ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
            FrameRender(wd, draw_data);
            FramePresent(wd);*/
            
        }

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
