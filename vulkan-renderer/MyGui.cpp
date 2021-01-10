#include "MyGui.h"
#include "VulkanRenderer.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Context.h"
#include "Window.h"
#include "ResourceManager.h"


MyGui::MyGui(VulkanRenderer *renderer)
{
	// https://frguthmann.github.io/posts/vulkan_imgui/
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	Window* window = renderer->GetVulkanContext()->GetWindow();
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

	// Upload Fonts
	{
		vk::CommandBuffer command_buffer = renderer->GetResourceManager()->BeginSingleTimeCommand();
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		renderer->GetResourceManager()->EndSingleTimeCommand(command_buffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}


MyGui::~MyGui()
{
}

void MyGui::Draw(vk::CommandBuffer& commandBuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

    // create window called graphics status.
	ImGui::Begin("graphics status");

    for (int i = 0; i < _graphicsInfos.size(); ++i)
    {
        ImGui::Text(_graphicsInfos[i].c_str());
    }

	ImGui::End();

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
}

void MyGui::Clear()
{
    _graphicsInfos.clear();
}

void MyGui::AddInfo(std::string info)
{
    _graphicsInfos.push_back(info);
}
