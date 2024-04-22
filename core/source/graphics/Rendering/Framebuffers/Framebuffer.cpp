#include "Framebuffer.h"

#include <graphics/Rendering/Utils/RenderPipelineFactory.h>

#include "../Textures/TextureFactory.h"


Framebuffer::Framebuffer(Resolution resolution, VulkanAPI::QueueFamily queueFamily)
	: resolution{resolution}, queueFamily{ queueFamily }
{
	Create();
}

Framebuffer::~Framebuffer()
{
	// Destroy the framebuffer
	-VulkanAPI::DestroyFramebuffer(framebuffer);
	// Destroy Render Pass
	-VulkanAPI::DestroyRenderPass(renderPass);
	
	// Destroy the attachments
	for (auto& attachment : attachments) {
		TextureFactory::DestroyTexture2D(attachment);
	}
}


void Framebuffer::CreateRenderPass()
{
	std::vector<VkAttachmentDescription> renderPassAttachments
	{
		// Pass 0
		{
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		}
	};


	VulkanAPI::CreateRenderPass(renderPassAttachments, renderPass);

}

void Framebuffer::Create()
{
	// create attachments.
	CreateRenderPass();
	CreateAttachments();

	// aquire the views to the framebuffer attachments
	std::vector<VkImageView> views;
	std::transform(attachments.begin(), attachments.end(), std::back_inserter(views), [](Texture2D* a) { return a->View(); });


	VulkanAPI::CreateFramebuffer(resolution.width, resolution.height, views, renderPass, framebuffer);

}

VkFramebuffer Framebuffer::Get()
{
	return framebuffer;
}

VkRenderPass Framebuffer::GetRenderPass() {
	return renderPass;
}

uint32_t Framebuffer::GetWidth()
{
	return resolution.width;
}

uint32_t Framebuffer::GetHeight()
{
	return resolution.height;
}

void Framebuffer::CreateAttachments() 
{
	// COLOR ATTACHMENT
	attachments.push_back(TextureFactory::CreateFramebufferTexture2D(resolution.width, resolution.height, TextureFormat::BGRA8_UNORM));

}
