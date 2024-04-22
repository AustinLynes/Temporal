#pragma once

#include <graphics/gfx_pch.h>
#include <datastructures/datastructures_pch.h>


class RenderPipeline;

class Texture2D;
class Framebuffer {
public:
	Framebuffer(Resolution resolution, VulkanAPI::QueueFamily queueFamily);
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

	VkFramebuffer framebuffer;
	VkRenderPass renderPass;

	std::vector<Texture2D*> attachments;

	VulkanAPI::QueueFamily queueFamily;

};