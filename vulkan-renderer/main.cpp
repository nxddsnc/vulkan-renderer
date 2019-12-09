#include "renderer.h"
#include "Window.h"
#include <array>
#include <chrono>
#include <iostream>

constexpr double PI = 3.14159265358979323846;
constexpr double CIRCLE_RAD = PI * 2;
constexpr double CIRCLE_THIRD = CIRCLE_RAD / 3.0;
constexpr double CIRCLE_THIRD_1 = 0;
constexpr double CIRCLE_THIRD_2 = CIRCLE_THIRD;
constexpr double CIRCLE_THIRD_3 = CIRCLE_THIRD * 2;

int main()
{
	Window *window = new Window(WIDTH, HEIGHT, "Vulkan_Renderer");

	Renderer renderer(window);

    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    uint64_t frame_counter = 0;
    uint64_t fps = 0;

	while (renderer.Run())
	{
        ++frame_counter;
        if (last_time + std::chrono::seconds(1) < timer.now()) {
            last_time = timer.now();
            fps = frame_counter;
            frame_counter = 0;
            std::cout << "FPS: " << fps << std::endl;
        }
		renderer.DrawFrame();
	}
	//VkCommandPool commandPool;
	//VkCommandPoolCreateInfo commandPoolCreateInfo = {};

	//commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	//commandPoolCreateInfo.queueFamilyIndex = renderer.GetGraphicFamilyIndex();
	//vkCreateCommandPool(renderer.GetVulkanDevice(), &commandPoolCreateInfo, nullptr, &commandPool);

	//std::vector<VkCommandBuffer> commandBuffers(w->GetSwapchainImageCount());
	//for (uint32_t i = 0; i < commandBuffers.size(); ++i)
	//{
	//	VkCommandBuffer commandBuffer = {};
	//	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	//	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//	commandBufferAllocateInfo.commandPool = commandPool;
	//	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	//	commandBufferAllocateInfo.commandBufferCount = 1;
	//	vkAllocateCommandBuffers(renderer.GetVulkanDevice(), &commandBufferAllocateInfo, &commandBuffer);
	//	commandBuffers[i]= commandBuffer;
	//}

	//VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
	//VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	//semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	//vkCreateSemaphore(renderer.GetVulkanDevice(), &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
	//
	//
	//float color_rotator = 0.0f;
	//auto timer = std::chrono::steady_clock();
	//auto last_time = timer.now();
	//uint64_t frame_counter = 0;
	//uint64_t fps = 0;
	//while (renderer.Run())
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
	//	vkQueueSubmit(renderer.GetVulkanDeviceQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	//	// End render
	//	w->EndRender({ renderCompleteSemaphore });
	//}
	//vkQueueWaitIdle(renderer.GetVulkanDeviceQueue());
	//vkDestroySemaphore(renderer.GetVulkanDevice(), renderCompleteSemaphore, nullptr);
	//vkDestroyCommandPool(renderer.GetVulkanDevice(), commandPool, nullptr);
	return 0;
}

//auto device = renderer._device;
//auto queue = renderer._queue;

//VkFence fence;
//VkFenceCreateInfo fenceCreateInfo = {};
//fenceCreateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
//fenceCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

//VkSemaphore semaphore;
//VkSemaphoreCreateInfo semaphoreCreateInfo = {};
//semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

//VkCommandPoolCreateInfo	commandPoolCreateInfo = {};
//commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//commandPoolCreateInfo.queueFamilyIndex = renderer._graphicFamilyIndex;
//commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//VkCommandPool commandPool;
//vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

//VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
//commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//commandBufferAllocateInfo.commandPool = commandPool;
//commandBufferAllocateInfo.commandBufferCount = 2;
//commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//VkCommandBuffer commandBuffers[2];
//vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);

//{
//	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
//	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//
//	vkBeginCommandBuffer(commandBuffers[0], &commandBufferBeginInfo);

//	VkViewport viewport;
//	viewport.height = 512;
//	viewport.width = 512;
//	viewport.maxDepth = 1.0;
//	viewport.minDepth = 0.0;
//	viewport.x = 0.0;
//	viewport.y = 0.0;
//	vkCmdSetViewport(commandBuffers[0], 0, 1, &viewport);

//	vkEndCommandBuffer(commandBuffers[0]);
//}
//{
//	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
//	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

//	vkBeginCommandBuffer(commandBuffers[1], &commandBufferBeginInfo);

//	VkViewport viewport;
//	viewport.height = 512;
//	viewport.width = 512;
//	viewport.maxDepth = 1.0;
//	viewport.minDepth = 0.0;
//	viewport.x = 0.0;
//	viewport.y = 0.0;
//	vkCmdSetViewport(commandBuffers[1], 0, 1, &viewport);

//	vkEndCommandBuffer(commandBuffers[1]);
//}

//
//{
//	VkSubmitInfo submitInfo = {};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &(commandBuffers[0]);
//	submitInfo.signalSemaphoreCount = 1;
//	submitInfo.pSignalSemaphores = &semaphore;

//	vkCmdPipelineBarrier(commandBuffers[0],
//		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
//		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
//		0,
//		0, nullptr,
//		0, nullptr,
//		0, nullptr);
//	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
//}
//{
//	VkPipelineStageFlags flags[]{ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
//	VkSubmitInfo submitInfo = {};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &(commandBuffers[1]);
//	submitInfo.waitSemaphoreCount = 1;
//	submitInfo.pWaitSemaphores = &semaphore;
//	submitInfo.pWaitDstStageMask = flags;
//	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
//}

////auto ret = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

////vkQueueWaitIdle(queue);

//vkDestroyCommandPool(device, commandPool, nullptr);
//vkDestroyFence(device, fence, nullptr);
//vkDestroySemaphore(device, semaphore, nullptr);