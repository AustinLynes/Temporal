#include "RenderPipeline.h"


void RenderPipeline::Initillize(uint32_t width, uint32_t height)
{
	InternalResolution = { width, height };
	
	CreateDescriptorPool();
	CreateRenderPass();
	CreatePipeline();
}

void RenderPipeline::Cleanup()
{
	OnDestroyPipeline();

	VulkanAPI::DestroyRenderPass(renderPass);
	VulkanAPI::DestroyPipelineLayout(layout);
	VulkanAPI::DestroyPipeline(pipeline);

}

VkPipeline RenderPipeline::Get()
{
	return this->pipeline;
}

const Resolution RenderPipeline::GetInternalRenderResolution()
{
	return InternalResolution;
}

const VkRenderPass RenderPipeline::GetRenderPass()
{
	return renderPass;
}
