#include "Framebuffer.h"

#include <graphics/Rendering/Utils/RenderPipelineFactory.h>


Framebuffer::Framebuffer(VkDevice device, Resolution resolution, VulkanAPI::QueueFamily queueFamily)
	:device{ device }, resolution{resolution}, queueFamily{ queueFamily }
{
	Create();
}

Framebuffer::~Framebuffer()
{
	// Destroy the framebuffer
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyFramebuffer(device, buffer, nullptr);
		buffer = VK_NULL_HANDLE;
	}

	// Destroy the render pass
	if (renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device, renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
	}

	// Destroy the attachments
	for (auto& attachment : attachments) {
		if (attachment.view != VK_NULL_HANDLE) {
			vkDestroyImageView(device, attachment.view, nullptr);
			attachment.view = VK_NULL_HANDLE;
		}
		if (attachment.image != VK_NULL_HANDLE) {
			vkDestroyImage(device, attachment.image, nullptr);
			attachment.image = VK_NULL_HANDLE;
		}
		if (attachment.memory != VK_NULL_HANDLE) {
			vkFreeMemory(device, attachment.memory, nullptr);
			attachment.memory = VK_NULL_HANDLE;
		}
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

	std::vector<VkAttachmentReference> attachmentRefs{
		// Pass 0 ref
		{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		}
	};

	std::vector<VkSubpassDescription> subpasses
	{
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = (uint32_t)attachmentRefs.size(),
			.pColorAttachments = attachmentRefs.data()
		}
	};


	VkRenderPassCreateInfo renderPassCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,

		.attachmentCount = (uint32_t)renderPassAttachments.size(),
		.pAttachments = renderPassAttachments.data(),

		.subpassCount = (uint32_t)subpasses.size(),
		.pSubpasses = subpasses.data(),

	};

	VK_CHECK(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass));
}

void Framebuffer::Create()
{
	// create attachments.
	CreateRenderPass();
	CreateAttachments();

	// aquire the views to the framebuffer attachments
	std::vector<VkImageView> views;
	std::transform(attachments.begin(), attachments.end(), std::back_inserter(views), [](const FramebufferAttachment& a) { return a.view; });

	VkFramebufferCreateInfo info
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		
		.renderPass = renderPass,

		.attachmentCount = (uint32_t) views.size(),
		.pAttachments = views.data(),

		.width = resolution.width,
		.height = resolution.height,

		.layers = 1
		
	};

	VK_CHECK(vkCreateFramebuffer(device, &info, nullptr, &buffer));

}

VkFramebuffer Framebuffer::Get()
{
	return buffer;
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
	FramebufferAttachment color{nullptr};
	{
		uint32_t gq_index;

		if (queueFamily.graphics.has_value())
			gq_index = queueFamily.graphics.value();

		uint32_t depth = 1;

		VkImageCreateInfo imageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.extent =
				{
					.width = resolution.width,
					.height = resolution.height,
					.depth = depth
				},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,

			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &gq_index,

			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		// create image
		VK_CHECK(vkCreateImage(device, &imageCreateInfo, nullptr, &color.image));

		
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device, color.image, &memReqs);

		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 
			.pNext = nullptr, 
			.allocationSize = memReqs.size,
			.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		};

		VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &color.memory));

		VK_CHECK(vkBindImageMemory(device, color.image, color.memory, 0));

		// create view
		VkImageViewCreateInfo viewCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = color.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = VK_REMAINING_MIP_LEVELS,
				.baseArrayLayer = 0,
				.layerCount = 1,

				}
		};

		VK_CHECK(vkCreateImageView(device, &viewCreateInfo, nullptr, &color.view));
	}

	attachments.push_back(color);
}
