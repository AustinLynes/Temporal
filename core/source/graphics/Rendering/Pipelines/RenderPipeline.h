#pragma once

#include "pch.h"

#include <graphics/gfx_pch.h>

class RenderPipeline {

public:
	RenderPipeline() = default;
	virtual ~RenderPipeline() = default;

	void Initillize(uint32_t width, uint32_t height);
	void Cleanup();
	void LinkDevice(VkDevice device);

	const Resolution GetInternalRenderResolution();
	const VkRenderPass GetRenderPass();

	VkPipeline Get();

protected: /* INTERFACE */
	virtual void CreateDescriptorPool() = 0;

	virtual void CreateRenderPass() = 0;
	virtual void CreatePipeline() = 0;

	virtual void OnDestroyPipeline() = 0;

protected:
	VkDevice device;
	Resolution InternalResolution;

	VkRenderPass renderPass;

	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkPipelineCache cache;


};

