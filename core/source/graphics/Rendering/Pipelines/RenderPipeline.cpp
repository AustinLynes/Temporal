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

	vkDestroyRenderPass(device, renderPass, nullptr);

	vkDestroyPipelineLayout(device, layout, nullptr);
	vkDestroyPipeline(device, pipeline, nullptr);


}

VkPipeline RenderPipeline::Get()
{
	return this->pipeline;
}

void RenderPipeline::LinkDevice(VkDevice dev)
{
	this->device = dev;
}

const Resolution RenderPipeline::GetInternalRenderResolution()
{
	return InternalResolution;
}

const VkRenderPass RenderPipeline::GetRenderPass()
{
	return renderPass;
}
