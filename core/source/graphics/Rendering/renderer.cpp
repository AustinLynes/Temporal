#include "renderer.h"


#include "Graphics/Rendering/Utils/RenderPipelineFactory.h"
#include "graphics/Rendering/Pipelines/RenderPipelines.h"
#include "Graphics/Rendering/Shaders/ShaderGraph.h"
#include "graphics/Rendering/Framebuffers/Framebuffer.h"
#include "graphics/Rendering/Swapchain/Swapchain.h"
#include "graphics/CommandManager.h"

#include <filesystem>
#include <debug/Console.h>




#include <array>

#pragma pack(push, 4)
struct SQVertex {
	float POS[3];
	float UV[2];
};
#pragma pack(pop)

constexpr std::array<SQVertex, 4> screenQuadVertices = {
	// Positions          // Texture coordinates
	SQVertex{-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,},
	SQVertex{ 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,},
	SQVertex{-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,},
	SQVertex{ 1.0f,  1.0f, 0.0f,  1.0f, 1.0f }
};

namespace r {
	VulkanAPI::QueueFamily QueueFamily;
	VulkanAPI::FenceBlock Fences;
	VulkanAPI::SemaphoreBlock Semaphores;

	uint32_t currentFrameIndex;
	VkSurfaceKHR Surface;

	namespace screen {
		VkBuffer vbo;
		VkBufferView vbo_View;
		VkDeviceMemory bufferMemory;
	}

}

#define PIPELINE_STR(pipe) #pipe

Renderer::Renderer() : window{nullptr}
{

}

Renderer::~Renderer()
{

}

TReturn Renderer::Initilize(GLFWwindow* window)
{
	this->window = window;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	Resolution resoulution;
	resoulution.width = static_cast<uint32_t>(width);
	resoulution.height = static_cast<uint32_t>(height);
	
	glfwSetWindowUserPointer(window, (void*)this);

	-VulkanAPI::GetRequiredInfo();

	// Create Instance 
	-VulkanAPI::CreateInstance();
	// Create Surface
	-VulkanAPI::CreateSurfaceGLFW(window, r::Surface);
	// Select A Physical Device
	-VulkanAPI::GetPhysicalDevice();
	// Create Device

	-VulkanAPI::ReserveQueueFamily(r::QueueFamily, r::Surface);

	-VulkanAPI::CreateDevice(r::QueueFamily);

	-VulkanAPI::CreateFenceBlock(r::Fences);
	-VulkanAPI::CreateSemaphoreBlock(r::Semaphores);

	commandManager = new CommandManager(r::QueueFamily);

	swapchain = new Swapchain(window, r::Surface, r::QueueFamily, resoulution, 3);

	framebuffer = new Framebuffer(resoulution, r::QueueFamily);

	//vk::renderPipelines.emplace(RenderPipelines::ScreenRenderPass, RenderPipelineFactory::Create<RenderPipelines::Basic2D>(vk::Device, resoulution));
	renderPipelines.emplace("Basic2D", RenderPipelineFactory::Create<RenderPipelines::Basic2D>(resoulution));
	
	// load some models...

	auto size = sizeof(SQVertex) * screenQuadVertices.size();
	-VulkanAPI::CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, r::screen::vbo);


	// bind to the window a resizing event
	
	glfwSetFramebufferSizeCallback(window, HandleResize);

	return TReturn::SUCCESS;
}

// allocate a command buffer to record commands to.
// begin recording process on allocated command buffer
// ... 
// record commands
// ...
// end recording process on allocated command buffer.
// submit command buffer 



void Renderer::RenderFrame() {
	

	VkCommandBuffer cmd = commandManager->BeginSingleTimeCommand(VulkanAPI::CommandType::Graphics);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipelines["Basic2D"]->Get());

	VkClearValue clearValues[2];
	clearValues[0].color = { {0.21f, 0.21f, 0.21f, 0} }; // Start color of the gradient

	
	VkRenderPassBeginInfo renderPassBegineInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = framebuffer->GetRenderPass(),
		.framebuffer = framebuffer->Get(),
		.renderArea = { 0, 0, framebuffer->GetWidth(), framebuffer->GetHeight() },
		.clearValueCount = _countof(clearValues), 
		.pClearValues = clearValues,
		
	};

	auto currentImage = swapchain->GetImage(r::currentFrameIndex);
	
	VkImageSubresourceRange subresourceRange
	{ VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,  VK_REMAINING_ARRAY_LAYERS };

	VkImageMemoryBarrier barrier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = currentImage,
		.subresourceRange = subresourceRange
	};

	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vkCmdClearColorImage(cmd, currentImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValues[0].color, 1,&subresourceRange );
	
	vkCmdBeginRenderPass(cmd, &renderPassBegineInfo, VK_SUBPASS_CONTENTS_INLINE);
	//vkCmdBindVertexBuffers(cmd, 0, );
	//vkCmdDraw(cmd, 4, 0, 0, 0);

	vkCmdEndRenderPass(cmd);

	VkSubmitInfo submit{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
	};

	commandManager->EndSingleTimeCommand(cmd, VulkanAPI::CommandType::Graphics, r::Fences.Drawing);

	Console::Log("Drawing Completed!");
}

void Renderer::PresentFrame()
{
	swapchain->AquireNextImage(r::Semaphores.ImageAvailable, r::currentFrameIndex);


	auto cmd = commandManager->BeginSingleTimeCommand(VulkanAPI::CommandType::Graphics);

	auto swapchain_ref = swapchain->Get();

	auto currentImage = swapchain->GetImage(r::currentFrameIndex);


	auto presentationQueue = commandManager->GetPresentQueue();

	VkImageMemoryBarrier barrier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = currentImage,
		.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
	};
	
	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0, 
		0, nullptr, 
		0, nullptr, 
		1, &barrier
		);

	commandManager->EndSingleTimeCommand(cmd, VulkanAPI::CommandType::Graphics, r::Fences.Presenting);

	VkPresentInfoKHR present
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, 
		.pNext = nullptr, 
		.waitSemaphoreCount = 1, 
		.pWaitSemaphores = &r::Semaphores.ImageAvailable,
		.swapchainCount = 1,
		.pSwapchains = &swapchain_ref,
		.pImageIndices = &r::currentFrameIndex,

	};

	VkResult res = vkQueuePresentKHR(presentationQueue, &present);


}

// 1. Get the new surface size
// 2. Recreate the swapchain with the new size
// 3. Recreate framebuffers associated with the new swapchain
// 4. Rebuild any resources dependent on the swapchain (pipelines, command buffers, etc.)
// 5. Reset or reinitialize synchronization objects if needed

void Renderer::HandleResize(GLFWwindow* win, int width, int height)
{
	auto rend = (Renderer*)glfwGetWindowUserPointer(win);


	std::string title = "Temporal [Vulkan]-(" + std::to_string(width) + ", " + std::to_string(height) + ")";
	glfwSetWindowTitle(win, title.c_str());

	Resolution res;
	res.width = static_cast<uint32_t>(width);
	res.height = static_cast<uint32_t>(height);


	delete rend->framebuffer;
	delete rend->swapchain;
	
	VulkanAPI::FreeSurface(r::Surface);

	VulkanAPI::CreateSurfaceGLFW(win, r::Surface);
	
	rend->swapchain = new Swapchain(win, r::Surface, r::QueueFamily, res, 3);

	rend->framebuffer = new Framebuffer(res, r::QueueFamily);




}


void Renderer::Cleanup()
{
	for (const auto& entry : renderPipelines)
	{
		auto pipeline = entry.second;
		pipeline->Cleanup();
		delete pipeline;
	}

	delete framebuffer;
	delete swapchain;
	delete commandManager;


	VulkanAPI::FreeFenceBlock(r::Fences);
	VulkanAPI::FreeSemaphoreBlock(r::Semaphores);

	VulkanAPI::FreeDevice();
	VulkanAPI::FreeSurface(r::Surface);
	VulkanAPI::FreeInstance();
}





