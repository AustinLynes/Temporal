#pragma once

#include <graphics/gfx_pch.h>
#include <datastructures/datastructures_pch.h>


class RenderPipeline;



struct FramebufferAttachment {
	VkImage image;
	VkDeviceMemory memory;

	VkImageView view;
};

class Framebuffer {
public:
	Framebuffer(VkDevice device, Resolution resolution, VulkanAPI::QueueFamily queueFamily);
	~Framebuffer();

	void Create();

	VkFramebuffer Get();
	VkRenderPass GetRenderPass();
	
	uint32_t GetWidth();
	uint32_t GetHeight();

private:
	void CreateRenderPass();
	void CreateAttachments();

	Resolution resolution;

	VkFramebuffer buffer;
	VkRenderPass renderPass;
	VkDevice device;

	std::vector<FramebufferAttachment> attachments;
	VulkanAPI::QueueFamily queueFamily;

};