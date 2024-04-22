#pragma once

#define T_RENDER_PIPELINE

#include "Graphics/Rendering/Pipelines/RenderPipeline.h"
#include "RenderPipelineUtils.h"

namespace RenderPipelineFactory {
	template<typename T>
	static RenderPipeline* Create(uint32_t width, uint32_t height) {
		auto pPipeline = Utils::Create<T>();
		pPipeline->Initillize(width, height);
		return pPipeline;
	}

	template<typename T>
	static RenderPipeline* Create(Resolution resolution) {
		return Create<T>(resolution.width, resolution.height);
	}

	static void Destroy(RenderPipeline* pPipeline) {
		pPipeline->Cleanup();
		delete pPipeline;
	}
}