#include "renderer.h"


#include "Graphics/Rendering/Utils/RenderPipelineFactory.h"
#include "graphics/Rendering/Pipelines/RenderPipelines.h"
#include "Graphics/Rendering/Shaders/ShaderGraph.h"
#include "graphics/Rendering/Framebuffers/Framebuffer.h"
#include "graphics/Rendering/Swapchain/Swapchain.h"
#include "graphics/CommandManager.h"

#include <filesystem>
#include <debug/Console.h>


namespace vk
{
	VkInstance Instance = { VK_NULL_HANDLE };
	VkPhysicalDevice PhysicalDevice = { VK_NULL_HANDLE };
	VkDevice Device = { VK_NULL_HANDLE };

	VulkanAPI::QueueFamily QueueFamily;

	CommandManager* commandManager;
	Framebuffer* framebuffer;
	Swapchain* swapchain;

	VulkanAPI::FenceBlock Fences;
	VulkanAPI::SemaphoreBlock Semaphores;

	VkSurfaceKHR Surface;
	uint32_t CurrentFrameIndex = 0;

	std::unordered_map<std::string, RenderPipeline*> renderPipelines;

	std::vector<std::string> layers = {};
	std::vector<std::string> instance_extensions = {};
	std::vector<std::string> device_extensions = {};

}// GLOBALS




Renderer::Renderer() : window{nullptr}
{

}

Renderer::~Renderer()
{

}

bool Renderer::Initilize(GLFWwindow* window)
{
	this->window = window;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	Resolution resoulution;
	resoulution.width = static_cast<uint32_t>(width);
	resoulution.height = static_cast<uint32_t>(height);

	GetRequiredInfo();

	// Create Instance 
	vk::Instance = VulkanAPI::CreateInstance(vk::layers, vk::instance_extensions);
	// Create Surface
	vk::Surface = VulkanAPI::CreateSurfaceGLFW(vk::Instance, window);
	// Select A Physical Device
	vk::PhysicalDevice = VulkanAPI::GetPhysicalDevice(vk::Instance);
	// Create Device
	vk::QueueFamily = VulkanAPI::ReserveQueueFamily(vk::PhysicalDevice, vk::Surface);
	vk::Device = VulkanAPI::CreateDevice(vk::Instance, vk::PhysicalDevice, vk::layers, vk::device_extensions, vk::QueueFamily);

	vk::Fences = VulkanAPI::CreateFenceBlock(vk::Device);
	vk::Semaphores = VulkanAPI::CreateSemaphoreBlock(vk::Device);

	vk::commandManager = new CommandManager(vk::Device, vk::QueueFamily);

	vk::swapchain = new Swapchain(vk::Device, vk::PhysicalDevice, window, vk::Surface, vk::QueueFamily, resoulution, 3);

	vk::framebuffer = new Framebuffer(vk::Device, resoulution, vk::QueueFamily);

	vk::renderPipelines.emplace("Basic2D", RenderPipelineFactory::Create<RenderPipelines::Basic2D>(vk::Device, resoulution));
		

	// bind to the window a resizing event
	
	glfwSetFramebufferSizeCallback(window, HandleResize);

	return vk::Instance != VK_NULL_HANDLE && vk::PhysicalDevice != VK_NULL_HANDLE && vk::Device != VK_NULL_HANDLE;
}

// allocate a command buffer to record commands to.
// begin recording process on allocated command buffer
// ... 
// record commands
// ...
// end recording process on allocated command buffer.
// submit command buffer 

using namespace VulkanAPI;

void Renderer::RenderFrame() {

	VkCommandBuffer cmd = vk::commandManager->BeginSingleTimeCommand(CommandType::Graphics);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk::renderPipelines["Basic2D"]->Get());
	//
	VkClearValue clearColor = { {{1.0f, 0.0f, 0.0f, 1.0f}} };
	
	VkRenderPassBeginInfo renderPass
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = vk::framebuffer->GetRenderPass(),
		.framebuffer = vk::framebuffer->Get(),
		.renderArea = { 0, 0, vk::framebuffer->GetWidth(), vk::framebuffer->GetHeight() },
		.clearValueCount = 1, 
		.pClearValues = &clearColor,
		
	};

	auto currentImage = vk::swapchain->GetImage(vk::CurrentFrameIndex);
	
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

	vkCmdClearColorImage(cmd, currentImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor.color, 1, &subresourceRange);
	
	vkCmdBeginRenderPass(cmd, &renderPass, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(cmd);

	VkSubmitInfo submit{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
	};

	vk::commandManager->EndSingleTimeCommand(cmd, CommandType::Graphics, vk::Fences.Drawing);

	while (vkGetFenceStatus(vk::Device, vk::Fences.Drawing) != VK_SUCCESS) {
		Console::Log("Waiting For Drawing To Complete...");
	};
	Console::Log("Drawing Completed!");
}

void Renderer::PresentFrame()
{
	VkResult res = vkAcquireNextImageKHR(vk::Device, vk::swapchain->Get(), UINT64_MAX, vk::Semaphores.ImageAvailable, VK_NULL_HANDLE, &vk::CurrentFrameIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		return;
	}
	else if (res != VK_SUCCESS) {
		// ERROR
	}
	auto cmd = vk::commandManager->BeginSingleTimeCommand(CommandType::Graphics);

	auto swapchain_ref = vk::swapchain->Get();

	auto currentImage = vk::swapchain->GetImage(vk::CurrentFrameIndex);


	auto presentationQueue = vk::commandManager->GetPresentQueue();

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

	vk::commandManager->EndSingleTimeCommand(cmd, CommandType::Graphics, vk::Fences.Presenting);

	VkPresentInfoKHR present
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, 
		.pNext = nullptr, 
		.waitSemaphoreCount = 1, 
		.pWaitSemaphores = &vk::Semaphores.ImageAvailable,
		.swapchainCount = 1,
		.pSwapchains = &swapchain_ref,
		.pImageIndices = &vk::CurrentFrameIndex,

	};

	res = vkQueuePresentKHR(presentationQueue, &present);


}

// 1. Get the new surface size
// 2. Recreate the swapchain with the new size
// 3. Recreate framebuffers associated with the new swapchain
// 4. Rebuild any resources dependent on the swapchain (pipelines, command buffers, etc.)
// 5. Reset or reinitialize synchronization objects if needed

void Renderer::HandleResize(GLFWwindow* win, int width, int height)
{
	std::string title = "Temporal [Vulkan]-(" + std::to_string(width) + ", " + std::to_string(height) + ")";
	glfwSetWindowTitle(win, title.c_str());

	Resolution res;
	res.width = static_cast<uint32_t>(width);
	res.height = static_cast<uint32_t>(height);


	delete vk::framebuffer;
	delete vk::swapchain;
	
	VulkanAPI::FreeSurface(vk::Instance, vk::Surface);

	vk::Surface = VulkanAPI::CreateSurfaceGLFW(vk::Instance, win);
	
	vk::swapchain = new Swapchain(vk::Device, vk::PhysicalDevice, win, vk::Surface, vk::QueueFamily, res, 3);
	
	vk::framebuffer = new Framebuffer(vk::Device, res, vk::QueueFamily);



}


void Renderer::Cleanup()
{
	for (const auto& entry : vk::renderPipelines)
	{
		auto pipeline = entry.second;
		pipeline->Cleanup();
		delete pipeline;
	}

	delete vk::framebuffer;
	delete vk::swapchain;
	delete vk::commandManager;


	VulkanAPI::FreeFenceBlock(vk::Device, vk::Fences);
	VulkanAPI::FreeSemaphoreBlock(vk::Device, vk::Semaphores);

	VulkanAPI::FreeDevice(vk::Device);
	VulkanAPI::FreeSurface(vk::Instance, vk::Surface);
	VulkanAPI::FreeInstance(vk::Instance);
}

void Renderer::GetRequiredInfo()
{
#ifdef _DEBUG 
	bool isValidationEnabled = true;
#else
	bool isValidationEnabled = false;
#endif


	if (isValidationEnabled)
	{
		vk::layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	// load required extension for use with GLFW
	uint32_t extCount = -1;
	auto glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&extCount);
	for (size_t i = 0; i < extCount; i++)
	{
		vk::instance_extensions.push_back(glfwRequiredExtensions[i]);
	}

	// load requried device extensions

	vk::device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}



