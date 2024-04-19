#pragma once


#include "Graphics/Rendering/Utils/RenderPipelineFactory.h"
#include "Graphics/Rendering/Shaders/ShaderGraph.h"

#include <filesystem>
#include <debug/Console.h>

namespace RenderPipelines {

	struct Vertex {
		struct {
			float x, y, z = 0;
		}Position;
		struct {
			float r, g, b, a = 0;
		}Color;
	};

	class Basic2D : public RenderPipeline {

		ShaderGraph* vertex;
		ShaderGraph* fragment;

		void CreateShaders() {

			// Crate Shader Module and Compile to SPIR-V From File
			auto baseDir = std::filesystem::current_path() / "Shaders";
			auto vertexFilepath = baseDir / "vertex.hlsl";

			// describe what will happen in the vertex stage..
			vertex = new ShaderGraph(device, vertexFilepath.generic_string(), VK_SHADER_STAGE_VERTEX_BIT);
			vertex->AddInput(0, ShaderVarType::_VEC3_, "Position", 0);
			vertex->AddInput(1, ShaderVarType::_VEC3_, "Color", 0);

			vertex->AddOutput(0, ShaderVarType::_VEC4_, "FragColor");

			vertex->AddMain("\tgl_Position = vec4(Position, 1.0);\n\tFragColor = vec4(Color, 1.0);\n");

			if (!vertex->Compile())
			{
				Console::Warn("Shader Compilation Failed: ", vertexFilepath);
			}

			auto fragmentFilepath = baseDir / "fragment.hlsl";

			fragment = new ShaderGraph(device, fragmentFilepath.generic_string(), VK_SHADER_STAGE_FRAGMENT_BIT);
			fragment->AddInput(0, ShaderVarType::_VEC4_, "FragColor", 0);
			fragment->AddOutput(0, ShaderVarType::_VEC4_, "OutputColor");

			fragment->AddMain("\tOutputColor = FragColor;\n");

			if (!fragment->Compile())
			{
				Console::Warn("Shader Compilation Failed: ", &fragment);
			}

		}

		virtual void CreateRenderPass() override {
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

		virtual void CreatePipeline() override
		{
			VkPipelineRasterizationStateCreateInfo RS{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_TRUE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f
			};

			// Describe Shader Stages

			std::vector<VkVertexInputBindingDescription> bindings{
				{.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
			};

			CreateShaders();

			std::vector<VkPipelineShaderStageCreateInfo> stages =
			{
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_VERTEX_BIT,
					.module = vertex->Get(),
					.pName = "main",

				},
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.module = fragment->Get(),
					.pName = "main"
				}
			};


			auto attribs = vertex->GetAttributes();



			VkPipelineVertexInputStateCreateInfo VI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,

				.vertexBindingDescriptionCount = (uint32_t)bindings.size(),
				.pVertexBindingDescriptions = bindings.data(),
				.vertexAttributeDescriptionCount = (uint32_t)attribs.size(),
				.pVertexAttributeDescriptions = attribs.data()
			};


			VkPipelineLayoutCreateInfo layoutCreateInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
			};

			VK_CHECK(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &layout));


			VkPipelineInputAssemblyStateCreateInfo IA
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE,
			};

			std::vector<VkViewport> viewports
			{
				{
					.x = 0, .y = 0, .width = static_cast<float>(InternalResolution.width), .height = static_cast<float>(InternalResolution.height), .minDepth = 0, .maxDepth = 1
				}
			};

			std::vector<VkRect2D> scissors
			{
				{
					.offset = { 0, 0 },
					.extent = { InternalResolution.width, InternalResolution.height }
				}
			};

			VkPipelineViewportStateCreateInfo VS
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.viewportCount = (uint32_t)viewports.size(),
				.pViewports = viewports.data(),
				.scissorCount = (uint32_t)scissors.size(),
				.pScissors = scissors.data()
			};


			VkPipelineMultisampleStateCreateInfo MSS
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE,
				.minSampleShading = 0,
				.pSampleMask = nullptr,
				.alphaToCoverageEnable = VK_FALSE,
				.alphaToOneEnable = VK_FALSE,
			};


			std::vector<VkPipelineColorBlendAttachmentState> blendAttachments
			{
				{
					.blendEnable = VK_TRUE,

					.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
					.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					.colorBlendOp = VK_BLEND_OP_ADD,

					.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
					.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
					.alphaBlendOp = VK_BLEND_OP_ADD,

					.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
				}
			};

			VkPipelineColorBlendStateCreateInfo BLEND
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,

				.logicOpEnable = VK_FALSE,
				.logicOp = VK_LOGIC_OP_COPY,

				.attachmentCount = (uint32_t)blendAttachments.size(),
				.pAttachments = blendAttachments.data(),

			};



			VkGraphicsPipelineCreateInfo info{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stageCount = (uint32_t)stages.size(),
				.pStages = stages.data(),
				.pVertexInputState = &VI,
				.pInputAssemblyState = &IA,
				.pViewportState = &VS,
				.pRasterizationState = &RS,
				.pMultisampleState = &MSS,
				.pColorBlendState = &BLEND,
				.layout = layout,
				.renderPass = renderPass,
				.subpass = 0
			};

			VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));
		}

		virtual void OnDestroyPipeline() override {
			
			delete vertex;
			delete fragment;

		};
	};
}
