#include "renderer.h"
#include "Window.h"
#include <array>
#include <chrono>
#include <iostream>
#include <GLFW\glfw3.h>

Renderer *renderer;
Window *window;

void ResizeCallback(GLFWwindow* window, int width, int height)
{
	renderer->Resize(width, height);
}

void CloseCallback(GLFWwindow* _window)
{
	window->Close();
}

int main()
{
	window = new Window(WIDTH, HEIGHT, "Vulkan_Renderer");
	renderer = new Renderer(window);

	glfwSetWindowSizeCallback(window->GetGLFWWindow(), ResizeCallback);
	glfwSetWindowCloseCallback(window->GetGLFWWindow(), CloseCallback);

    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    uint64_t frame_counter = 0;
    uint64_t fps = 0;

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
	//VkCommandPool commandPool;
	//VkCommandPoolCreateInfo commandPoolCreateInfo = {};

	//commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	//commandPoolCreateInfo.queueFamilyIndex = renderer->GetGraphicFamilyIndex();
	//vkCreateCommandPool(renderer->GetVulkanDevice(), &commandPoolCreateInfo, nullptr, &commandPool);

	//std::vector<VkCommandBuffer> commandBuffers(w->GetSwapchainImageCount());
	//for (uint32_t i = 0; i < commandBuffers.size(); ++i)
	//{
	//	VkCommandBuffer commandBuffer = {};
	//	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	//	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//	commandBufferAllocateInfo.commandPool = commandPool;
	//	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//	commandBufferAllocateInfo.commandBufferCount = 1;
	//	vkAllocateCommandBuffers(renderer->GetVulkanDevice(), &commandBufferAllocateInfo, &commandBuffer);
	//	commandBuffers[i]= commandBuffer;
	//}

	//VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
	//VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	//semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	//vkCreateSemaphore(renderer->GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
	//
	//
	//float color_rotator = 0.0f;
	//auto timer = std::chrono::steady_clock();
	//auto last_time = timer.now();
	//uint64_t frame_counter = 0;
	//uint64_t fps = 0;
	//while (renderer->Run())
	//{
	//	// CPU logic calculations
	//	++frame_counter;
	//	if (last_time + std::chrono::seconds(1) < timer.now()) {
	//		last_time = timer.now();
	//		fps = frame_counter;
	//		frame_counter = 0;
	//		std::cout << "FPS: " << fps << std::endl;
	//	}
	//	// Begin render
	//	w->BeginRender();
	//	// Record command buffer

	//	for (uint32_t i = 0; i < commandBuffers.size(); ++i)
	//	{
	//		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	//		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//		vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);

	//		VkRect2D renderArea = {};
	//		renderArea.extent = w->GetVulkanSurfaceSize();
	//		renderArea.offset.x = 0;
	//		renderArea.offset.y = 0;

	//		std::array<VkClearValue, 2> clearValues{};
	//		clearValues[0].depthStencil.depth = 0.0f;
	//		clearValues[0].depthStencil.stencil = 0;

	//		color_rotator += 0.001;
	//		//FIXME: use appropriate fields here
	//		clearValues[1].color.float32[0] = std::sin(color_rotator + CIRCLE_THIRD_1) * 0.5 + 0.5;
	//		clearValues[1].color.float32[1] = std::sin(color_rotator + CIRCLE_THIRD_2) * 0.5 + 0.5;
	//		clearValues[1].color.float32[2] = std::sin(color_rotator + CIRCLE_THIRD_3) * 0.5 + 0.5;
	//		clearValues[1].color.float32[3] = 1.0f;
	//		VkRenderPassBeginInfo renderPassBeginInfo = {};
	//		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//		renderPassBeginInfo.renderPass = w->GetVulkanRenderPass();
	//		renderPassBeginInfo.framebuffer = w->GetActiveFramebuffer();
	//		renderPassBeginInfo.renderArea = renderArea;
	//		renderPassBeginInfo.clearValueCount = clearValues.size();
	//		renderPassBeginInfo.pClearValues = clearValues.data();
	//		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	//
	//		// Begin draw commands
	//		//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	//		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

	//		vkCmdEndRenderPass(commandBuffers[i]);
	//		vkEndCommandBuffer(commandBuffers[i]);
	//	}
	//	// Submit command buffer
	//	VkSubmitInfo submitInfo = {};
	//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//	submitInfo.waitSemaphoreCount = 0;
	//	submitInfo.pWaitSemaphores = nullptr;
	//	submitInfo.pWaitDstStageMask = nullptr;
	//	submitInfo.commandBufferCount = commandBuffers.size();
	//	submitInfo.pCommandBuffers = commandBuffers.data();
	//	submitInfo.signalSemaphoreCount = 1;
	//	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;
	//	vkQueueSubmit(renderer->GetVulkanDeviceQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	//	// End render
	//	w->EndRender({ renderCompleteSemaphore });
	//}
	//vkQueueWaitIdle(renderer->GetVulkanDeviceQueue());
	//vkDestroySemaphore(renderer->GetVulkanDevice(), renderCompleteSemaphore, nullptr);
	//vkDestroyCommandPool(renderer->GetVulkanDevice(), commandPool, nullptr);

	delete renderer;
	return 0;
}
